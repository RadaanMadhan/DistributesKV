#include "replication_coordinator.hpp"

void ReplicationCoordinator::put(std::string& key, VersionedValue& value){
    auto replicas = request_router_.GetReplicas(key, replication_factor_);
    for (auto& node : replicas) {
        
    }
}