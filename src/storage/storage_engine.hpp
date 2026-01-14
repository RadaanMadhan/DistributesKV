#pragma once

#include <string>
#include <optional>
#include <unordered_map>
#include <mutex>
#include "../common/versioned_value.hpp"

class StorageEngine {
private:
  std::unordered_map<std::string, VersionedValue> storage_;
  std::mutex mutex_;
public:
  bool put(const std::string& key, const VersionedValue& value);
  std::optional<VersionedValue> get(const std::string& key);
};