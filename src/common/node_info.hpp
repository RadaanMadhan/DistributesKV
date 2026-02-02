#pragma once

#include <string>

struct NodeInfo{
    std::string id_;
    std::string host_;
    int port_;

    NodeInfo() : id_(""), host_(""), port_(0) {}
    NodeInfo(std::string id, std::string host, int port)
        : id_(std::move(id)), host_(std::move(host)), port_(port) {}

    bool operator==(const NodeInfo n)const{
        return id_ == n.id_;
    }
};