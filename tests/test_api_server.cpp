#include <gtest/gtest.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <cstring>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "../src/api/api.hpp"
#include "../src/common/thread_pool.hpp"
#include "../src/common/node_info.hpp"
#include "../src/replication/irpc.hpp"
#include "../src/replication/replication_coordinator.hpp"
#include "../src/router/request_router.hpp"
#include "../src/storage/storage_engine.hpp"

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

int connect_with_retry(const std::string& host, int port, int attempts = 20) {
    for (int i = 0; i < attempts; ++i) {
        int fd = connect_to_server(host, port);
        if (fd >= 0) {
            return fd;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    return -1;
}

bool start_on_free_port(APIServer& server, const std::string& host, int& port_out) {
    for (int port = 15000; port < 15100; ++port) {
        if (server.start(host, port)) {
            port_out = port;
            return true;
        }
    }
    return false;
}
} // namespace

TEST(ApiServerTest, PutThenGet) {
    int num_nodes = 3;
    int replication_factor = 3;
    int write_quorum = 2;
    int read_quorum = 2;
    int num_threads = 2;
    int vnodes_per_node = 5;

    std::vector<NodeInfo> nodes;
    nodes.reserve(num_nodes);
    for (int i = 0; i < num_nodes; ++i) {
        nodes.push_back(NodeInfo{
            "node" + std::to_string(i),
            "127.0.0.1",
            9000 + i
        });
    }

    ThreadPool thread_pool(num_threads);
    IRPCClientLocal irpc_client;

    std::vector<std::unique_ptr<StorageEngine>> storage_engines;
    storage_engines.reserve(num_nodes);
    for (const auto& node : nodes) {
        storage_engines.push_back(std::make_unique<StorageEngine>());
        irpc_client.registerNode(node.id_, storage_engines.back().get());
    }

    RequestRouter request_router(nodes, vnodes_per_node);
    ReplicationCoordinator replication_coordinator(
        replication_factor,
        write_quorum,
        read_quorum,
        request_router,
        thread_pool,
        irpc_client
    );

    APIServer api_server(replication_coordinator, nodes.front().id_);
    int port = 0;
    ASSERT_TRUE(start_on_free_port(api_server, "127.0.0.1", port));

    int fd = connect_with_retry("127.0.0.1", port);
    ASSERT_GE(fd, 0);

    ASSERT_TRUE(send_all(fd, "PUT alpha 123\n"));
    std::string response;
    ASSERT_TRUE(recv_line(fd, response));
    EXPECT_EQ(response, "OK");

    ASSERT_TRUE(send_all(fd, "GET alpha\n"));
    ASSERT_TRUE(recv_line(fd, response));
    EXPECT_TRUE(response.rfind("VALUE ", 0) == 0);

    ::shutdown(fd, SHUT_RDWR);
    ::close(fd);

    api_server.stop();
}
