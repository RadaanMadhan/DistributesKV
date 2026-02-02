#include "api.hpp"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <sstream>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

namespace {
std::string trim(const std::string& s) {
	auto start = s.find_first_not_of(" \t\r\n");
	if (start == std::string::npos) {
		return "";
	}
	auto end = s.find_last_not_of(" \t\r\n");
	return s.substr(start, end - start + 1);
}

uint64_t now_millis() {
	using namespace std::chrono;
	return static_cast<uint64_t>(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
}

bool send_all(int fd, const std::string& data) {
	size_t total = 0;
	while (total < data.size()) {
		auto sent = ::send(fd, data.data() + total, data.size() - total, 0);
		if (sent <= 0) {
			return false;
		}
		total += static_cast<size_t>(sent);
	}
	return true;
}
} // namespace

APIServer::APIServer(ReplicationCoordinator& coordinator, std::string node_id)
	: coordinator_(coordinator), node_id_(std::move(node_id)) {}

APIServer::~APIServer() {
	stop();
}

bool APIServer::start(const std::string& host, int port) {
	if (running_) {
		return false;
	}

	server_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd_ < 0) {
		return false;
	}

	int opt = 1;
	if (::setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		::close(server_fd_);
		server_fd_ = -1;
		return false;
	}

	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(static_cast<uint16_t>(port));
	if (host.empty() || host == "0.0.0.0") {
		addr.sin_addr.s_addr = INADDR_ANY;
	} else {
		if (::inet_pton(AF_INET, host.c_str(), &addr.sin_addr) <= 0) {
			::close(server_fd_);
			server_fd_ = -1;
			return false;
		}
	}

	if (::bind(server_fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
		::close(server_fd_);
		server_fd_ = -1;
		return false;
	}

	if (::listen(server_fd_, 128) < 0) {
		::close(server_fd_);
		server_fd_ = -1;
		return false;
	}

	running_ = true;
	accept_thread_ = std::thread(&APIServer::accept_loop, this);
	return true;
}

void APIServer::stop() {
	if (!running_) {
		return;
	}
	running_ = false;
	if (server_fd_ >= 0) {
		::shutdown(server_fd_, SHUT_RDWR);
		::close(server_fd_);
		server_fd_ = -1;
	}
	if (accept_thread_.joinable()) {
		accept_thread_.join();
	}
}

void APIServer::accept_loop() {
	while (running_) {
		sockaddr_in client_addr{};
		socklen_t client_len = sizeof(client_addr);
		int client_fd = ::accept(server_fd_, reinterpret_cast<sockaddr*>(&client_addr), &client_len);
		if (client_fd < 0) {
			if (!running_) {
				break;
			}
			continue;
		}

		std::thread(&APIServer::handle_client, this, client_fd).detach();
	}
}

void APIServer::handle_client(int client_fd) {
	std::string buffer;
	char temp[1024];
	while (true) {
		auto received = ::recv(client_fd, temp, sizeof(temp), 0);
		if (received <= 0) {
			break;
		}
		buffer.append(temp, temp + received);

		size_t pos = 0;
		while ((pos = buffer.find('\n')) != std::string::npos) {
			std::string line = trim(buffer.substr(0, pos));
			buffer.erase(0, pos + 1);
			if (line.empty()) {
				continue;
			}

			std::istringstream iss(line);
			std::string command;
			iss >> command;
			if (command == "PUT") {
				std::string key;
				iss >> key;
				std::string value;
				std::getline(iss, value);
				value = trim(value);

				if (key.empty() || value.empty()) {
					send_all(client_fd, "ERR invalid PUT\n");
					continue;
				}

				VersionedValue vv{value, now_millis(), node_id_};
				bool ok = coordinator_.put(key, vv);
				send_all(client_fd, ok ? "OK\n" : "ERR put failed\n");
			} else if (command == "GET") {
				std::string key;
				iss >> key;
				if (key.empty()) {
					send_all(client_fd, "ERR invalid GET\n");
					continue;
				}

				auto result = coordinator_.get(key);
				if (!result) {
					send_all(client_fd, "NOT_FOUND\n");
					continue;
				}

				const auto& vv = *result;
				std::ostringstream response;
				response << "VALUE " << vv.value << " " << vv.timestamp << " " << vv.node_id << "\n";
				send_all(client_fd, response.str());
			} else {
				send_all(client_fd, "ERR unknown command\n");
			}
		}
	}

	::shutdown(client_fd, SHUT_RDWR);
	::close(client_fd);
}
