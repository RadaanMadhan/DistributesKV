#pragma once

#include <string>
#include <vector>

#include "../common/node_info.hpp"

class RequestRouter{
public:
    explicit RequestRouter(std::vector<NodeInfo>nodes):nodes_(nodes){}

    std::vector<NodeInfo> GetReplicas(const std::string &key);
private:
    std::vector<NodeInfo> nodes_;

    std::string Hash(std::string key);
};