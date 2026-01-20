#pragma once

#include <string>

struct NodeInfo{
    std::string id;
    std::string host;
    int port;

    bool operator==(const NodeInfo n)const{
        return id == n.id;
    }
};