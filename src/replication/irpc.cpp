#include "irpc.hpp"

void IRPCClientLocal::registerNode(const std::string& node_id, StorageEngine* storage_engine){
    node_registry_[node_id] = storage_engine;
}

bool IRPCClientLocal::sendPut(NodeInfo& node_info, std::string key, VersionedValue& value){
    if (node_registry_.count(node_info.id_)){
        node_registry_[node_info.id_]->put(key, value);
        return true;
    }
    return false;
}

std::optional<VersionedValue> IRPCClientLocal::sendGet(NodeInfo& node_info, std::string key){
    if (node_registry_.count(node_info.id_)){
        return node_registry_[node_info.id_]->get(key);
    }
    return std::nullopt;
}