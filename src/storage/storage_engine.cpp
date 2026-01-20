#include "storage_engine.hpp"

#include "../common/exceptions.hpp"

// Store key value pair in unordered map
// Modification only occurs if key doesn't already exist or a more recent modification exists 
// Return True if modification suceeded, false if not
void StorageEngine::put(const std::string& key, const VersionedValue& value){
    std::scoped_lock(mutex_);
    auto it = storage_.find(key);
    if (it != storage_.end() && it->second > value) {
        throw StaleDataException("More Recent writes exist for key: " + key);
    }
    storage_[key] = value;
    
}

// Get value mapped to the key
// If key doesn't exist return nullopt
std::optional<VersionedValue> StorageEngine::get(const std::string& key){
    auto it = storage_.find(key);
    if (it == storage_.end()){
        return std::nullopt;
    }
    return it->second;
}
