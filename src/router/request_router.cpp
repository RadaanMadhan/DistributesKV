#include "request_router.hpp"

#include <cstdint>
#include <unordered_set>

#include "../common/exceptions.hpp"

const uint64_t SEED = 0x12345678;

RequestRouter::RequestRouter(std::vector<NodeInfo> nodes, int vnodes_per_node) { 
    for (auto &node : nodes){
        for (int vnode_num = 0; vnode_num < vnodes_per_node; vnode_num++){
            // Hash node_id + # + vnode_num for unique hashes to populate ring
            std::string vnode_id = node.id + "#" + std::to_string(vnode_num);
            ring_[Hash(vnode_id)] = node;
        } 
    } 
}

std::vector<NodeInfo>& RequestRouter::GetReplicas(const std::string &key, int num_replicas){
    if (node_count < num_replicas){
        throw ClusterSizeException("Number of active nodes is less than number of replication nodes requested");
    }

    std::vector<NodeInfo> preference_list;
    if (ring_.empty()) return preference_list;

    std::unordered_set<std::string> nodes_added;
    size_t key_hash = Hash(key);
    // Closest Node to key
    auto it = ring_.lower_bound(key_hash);
    while (preference_list.size() < num_replicas){
        // Loop around if at end of map
        if (it == ring_.end()){
            it = ring_.begin();
        }

        const NodeInfo& candidate = it->second;
        if (nodes_added.find(candidate.id) != nodes_added.end()){
            preference_list.push_back(candidate);
            nodes_added.insert(candidate.id);
        }

        // go to next node in clockwise direction
        ++it;
    }
    return preference_list;
}


size_t RequestRouter::Hash(std::string key){
    const uint8_t* data = reinterpret_cast<const uint8_t*>(key.data());
    const int len = static_cast<int>(key.length());
    const int nblocks = len / 16;

    uint64_t h1 = SEED;
    uint64_t h2 = SEED;

    const uint64_t c1 = 0x87c37b91114253d5ULL;
    const uint64_t c2 = 0x4cf5ad432745937fULL;

    // ---------- body
    const uint64_t* blocks = reinterpret_cast<const uint64_t*>(data);
    for (int i = 0; i < nblocks; i++) {
        uint64_t k1 = blocks[i * 2];
        uint64_t k2 = blocks[i * 2 + 1];

        k1 *= c1; k1 = (k1 << 31) | (k1 >> 33); k1 *= c2; h1 ^= k1;
        h1 = (h1 << 27) | (h1 >> 37); h1 = h1 * 5 + 0x52dce729;

        k2 *= c2; k2 = (k2 << 33) | (k2 >> 31); k2 *= c1; h2 ^= k2;
        h2 = (h2 << 31) | (h2 >> 33); h2 = h2 * 5 + 0x38495ab5;
    }

    // ---------- tail (handling remaining bytes)
    const uint8_t* tail = (const uint8_t*)(data + nblocks * 16);
    uint64_t k1 = 0;
    uint64_t k2 = 0;
    switch (len & 15) {
        case 15: k2 ^= (uint64_t)tail[14] << 48;
        case 14: k2 ^= (uint64_t)tail[13] << 40;
        case 13: k2 ^= (uint64_t)tail[12] << 32;
        case 12: k2 ^= (uint64_t)tail[11] << 24;
        case 11: k2 ^= (uint64_t)tail[10] << 16;
        case 10: k2 ^= (uint64_t)tail[9] << 8;
        case  9: k2 ^= (uint64_t)tail[8]; k2 *= c2; k2 = (k2 << 33) | (k2 >> 31); k2 *= c1; h2 ^= k2;
        case  8: k1 ^= (uint64_t)tail[7] << 56;
        case  7: k1 ^= (uint64_t)tail[6] << 48;
        case  6: k1 ^= (uint64_t)tail[5] << 40;
        case  5: k1 ^= (uint64_t)tail[4] << 32;
        case  4: k1 ^= (uint64_t)tail[3] << 24;
        case  3: k1 ^= (uint64_t)tail[2] << 16;
        case  2: k1 ^= (uint64_t)tail[1] << 8;
        case  1: k1 ^= (uint64_t)tail[0]; k1 *= c1; k1 = (k1 << 31) | (k1 >> 33); k1 *= c2; h1 ^= k1;
    };

    // ---------- finalization
    h1 ^= len; h2 ^= len;
    h1 += h2; h2 += h1;

    // Mixing (fmix64)
    auto fmix64 = [](uint64_t k) {
        k ^= k >> 33;
        k *= 0xff51afd7ed558ccdULL;
        k ^= k >> 33;
        k *= 0xc4ceb9fe1a85ec53ULL;
        k ^= k >> 33;
        return k;
    };

    h1 = fmix64(h1);
    h2 = fmix64(h2);
    h1 += h2;

    return static_cast<size_t>(h1);
}