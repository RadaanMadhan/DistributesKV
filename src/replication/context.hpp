#pragma once

#include <condition_variable>
#include <mutex>
#include <vector>

#include "../common/versioned_value.hpp"

struct WriteContext{
    std::mutex mutex_;
    std::condition_variable condition_;
    int success_count_ = 0;
    int failiure_count_ = 0;
    int total_replies_ = 0;
    int replication_factor_;
    int write_quorum_;

    WriteContext(int replication_factor, int write_quorum) : replication_factor_(replication_factor), write_quorum_(write_quorum){}
};

struct ReadContext{
    std::mutex mutex_;
    std::condition_variable condition_;
    std::vector<VersionedValue> versions_;
    int failiure_count_ = 0;
    int total_replies_ = 0;
    int replication_factor_;
    int read_quorum_;

    ReadContext(int replication_factor, int read_quorum) : replication_factor_(replication_factor), read_quorum_(read_quorum){}

};