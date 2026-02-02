#include "replication_coordinator.hpp"

#include "context.hpp"

#include <algorithm>

bool ReplicationCoordinator::put(std::string& key, VersionedValue& value){
    auto replicas = request_router_.GetReplicas(key, replication_factor_);
    auto ctx = std::make_shared<WriteContext>(replication_factor_, write_quorum_);
    for (auto& node : replicas) {
        auto key_copy = key;
        auto value_copy = value;
        thread_pool_.enqueue([this, node, ctx, key_copy, value_copy]() mutable {
            bool complete = irpc_.sendPut(node, key_copy, value_copy);
            {
                std::scoped_lock<std::mutex> lock(ctx->mutex_);
                if (complete) ctx->success_count_++;
                else ctx->failiure_count_++;
                ctx->total_replies_++; 
            }

            ctx->condition_.notify_one();
        });
    }
    std::unique_lock<std::mutex> lock(ctx->mutex_);
    bool finished = ctx->condition_.wait_for(lock, std::chrono::milliseconds(500), [&]{
        return ctx->success_count_ >= ctx->write_quorum_ || ctx->total_replies_ == ctx->replication_factor_;
    });
    if (finished && ctx->success_count_ >= ctx->write_quorum_){
        return true;
    }
    return false;
}


std::optional<VersionedValue> ReplicationCoordinator::get(std::string& key){
     auto replicas = request_router_.GetReplicas(key, replication_factor_);
    auto ctx = std::make_shared<ReadContext>(replication_factor_, read_quorum_);
    for (auto& node : replicas) {
        auto key_copy = key;
        thread_pool_.enqueue([this, node, ctx, key_copy]() mutable {
            std::optional<VersionedValue> value = irpc_.sendGet(node, key_copy);
            {
                std::scoped_lock<std::mutex> lock(ctx->mutex_);
                if (value.has_value()) {
                    ctx->versions_.push_back(*value);
                } else {
                    ctx->failiure_count_++;
                }
                ctx->total_replies_++; 
            }

            ctx->condition_.notify_one(); 
        });
    }  
    std::unique_lock<std::mutex> lock(ctx->mutex_);
    bool finished = ctx->condition_.wait_for(lock, std::chrono::milliseconds(500), [&]{
        return ctx->versions_.size() >= ctx->read_quorum_ || ctx->total_replies_ == ctx->replication_factor_;
    });

    if (!finished || ctx->versions_.size() < read_quorum_){
        return std::nullopt;
    }

    // TODO: Send most up to date value to nodes that did not retunrn it
   
    return *std::max_element(ctx->versions_.begin(), ctx->versions_.end());
}