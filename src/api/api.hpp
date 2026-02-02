#pragma once

#include <atomic>
#include <string>
#include <thread>

#include "../replication/replication_coordinator.hpp"

class APIServer{
public:
	APIServer(ReplicationCoordinator& coordinator, std::string node_id);
	~APIServer();

	bool start(const std::string& host, int port);
	void stop();

private:
	void accept_loop();
	void handle_client(int client_fd);

	ReplicationCoordinator& coordinator_;
	std::string node_id_;
	std::atomic<bool> running_{false};
	int server_fd_{-1};
	std::thread accept_thread_;
};