#include <iostream>
#include <memory>
#include <vector>

#include "api/api.hpp"
#include "replication/replication_coordinator.hpp"
#include "router/request_router.hpp"
#include "storage/storage_engine.hpp"
#include "replication/irpc.hpp"
#include "router/request_router.hpp"
#include "common/thread_pool.hpp"
#include "common/node_info.hpp"


int main(){
    int num_nodes = 20;
    int replication_factor = 10;
    int write_quorum = 3;
    int read_quorum = 3;
    int num_threads = 5;
    int vnodes_per_node = 10;

    std::vector<NodeInfo> nodes;
    nodes.reserve(num_nodes);
    for (int i = 0; i < num_nodes; ++i) {
        nodes.push_back(NodeInfo{
            "node" + std::to_string(i),
            "127.0.0.1",
            8000 + i
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
    if (!api_server.start("0.0.0.0", nodes.front().port_)) {
        std::cerr << "Failed to start API server" << std::endl;
        return 1;
    }

    std::cout << "API server listening on 0.0.0.0:" << nodes.front().port_ << std::endl;
    std::cout << "Press Enter to stop." << std::endl;
    std::string line;
    std::getline(std::cin, line);

    api_server.stop();
    return 0;

}