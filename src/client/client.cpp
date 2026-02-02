#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

namespace {
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

bool recv_line(int fd, std::string& out) {
	out.clear();
	char ch;
	while (true) {
		auto n = ::recv(fd, &ch, 1, 0);
		if (n <= 0) {
			return false;
		}
		if (ch == '\n') {
			return true;
		}
		out.push_back(ch);
	}
}

int connect_to_server(const std::string& host, int port) {
	int fd = ::socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		return -1;
	}

	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(static_cast<uint16_t>(port));
	if (::inet_pton(AF_INET, host.c_str(), &addr.sin_addr) <= 0) {
		::close(fd);
		return -1;
	}

	if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
		::close(fd);
		return -1;
	}

	return fd;
}

void usage() {
	std::cout << "Usage:\n"
			  << "  client <host> <port> PUT <key> <value>\n"
			  << "  client <host> <port> GET <key>\n";
}
} // namespace

int main(int argc, char** argv) {
	if (argc < 5) {
		usage();
		return 1;
	}

	std::string host = argv[1];
	int port = std::stoi(argv[2]);
	std::string command = argv[3];

	int fd = connect_to_server(host, port);
	if (fd < 0) {
		std::cerr << "Failed to connect to " << host << ":" << port << std::endl;
		return 1;
	}

	std::ostringstream request;
	if (command == "PUT") {
		if (argc < 6) {
			usage();
			::close(fd);
			return 1;
		}
		request << "PUT " << argv[4] << " " << argv[5] << "\n";
	} else if (command == "GET") {
		request << "GET " << argv[4] << "\n";
	} else {
		usage();
		::close(fd);
		return 1;
	}

	if (!send_all(fd, request.str())) {
		std::cerr << "Failed to send request" << std::endl;
		::close(fd);
		return 1;
	}

	std::string response;
	if (!recv_line(fd, response)) {
		std::cerr << "Failed to receive response" << std::endl;
		::close(fd);
		return 1;
	}

	std::cout << response << std::endl;
	::close(fd);
	return 0;
}
