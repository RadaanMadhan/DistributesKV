#include "request_router.hpp"


std::vector<NodeInfo> RequestRouter::GetReplicas(const std::string &key){
    std::string key_hash = Hash(key);
    std::vector<NodeInfo> result_nodes;
    for (auto node_info : nodes_){
        if (key_hash == node_info.id){

        }
    }
}
