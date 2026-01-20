#pragma once

#include <string>

#include "../storage/storage_engine.hpp"
#include "../router/request_router.hpp"
#include "../common/versioned_value.hpp"
#include "../common/thread_pool.hpp"

class ReplicationCoordinator{
public:
    explicit ReplicationCoordinator(int replication_factor, int write_quorum, int read_quorum, RequestRouter& request_router, ThreadPool& thread_pool) :
        replication_factor_(replication_factor),
        write_quorum_(write_quorum),
        read_quorum_(read_quorum), 
        request_router_(request_router),
        thread_pool_(thread_pool) 
        {}

    // attempts write to replication_factor nodes and waits for atleast write_quorum acknowledgements
    void put(std::string& key, VersionedValue& value);

    // atttempts read from replication_factor nodes and return the most recent data returned by atleast read_quorum nodes
    // updates lagging nodes to most recent data
    VersionedValue get(std::string& key);

private:
    int replication_factor_;
    int write_quorum_;
    int read_quorum_; 
    RequestRouter request_router_;
    ThreadPool& thread_pool_;
};