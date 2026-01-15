#pragma once

#include <string>
#include <vector>
#include <map>

#include "../common/node_info.hpp"

class RequestRouter{
public:
    explicit RequestRouter(std::vector<NodeInfo>nodes, int vnodes_per_nodes);

    // Get num_replicas closest nodes whose hash is less than hash of key 
    std::vector<NodeInfo> GetReplicas(const std::string &key, int num_replicas);
private:
    // Ring of hashes where node_ids sit on
    std::map<size_t, NodeInfo> ring_;
    int node_count;

    size_t Hash(std::string key);
};