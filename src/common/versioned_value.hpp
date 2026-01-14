#pragma once

#include <cstdint>
#include <string>

// Struct that formats value from KV pair to contatin information about timestamp and storage node that it is present in
struct VersionedValue {
    std::string value;
    uint64_t timestamp;
    std::string node_id;

    // Less than ovveride for VersionedValue
    // Return True if timestamp of first op is less than timestamp of second op
    bool operator<(VersionedValue const &v) const {
        if (timestamp != v.timestamp){
            return timestamp < v.timestamp;
        }
        // Tiebraker on node_id
        return node_id < v.node_id; 
    }

    // Greater than ovveride for VersionedValue
    // Return True if timestamp of first op is more than timestamp of second op
    bool operator>(VersionedValue const &v) const {
        if (timestamp != v.timestamp){
            return timestamp > v.timestamp;
        }
        // Tiebraker on node_id
        return node_id > v.node_id; 
    }
};