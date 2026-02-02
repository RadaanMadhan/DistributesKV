#pragma once

#include <map>
#include <optional>

#include "../common/versioned_value.hpp"
#include "../common/node_info.hpp"
#include "../storage/storage_engine.hpp"

class IRPCClient{
public:
    virtual ~IRPCClient() = default;
    virtual bool sendPut(NodeInfo& node_info, std::string key, VersionedValue& value) = 0;
    virtual std::optional<VersionedValue> sendGet(NodeInfo& node_info, std::string key) = 0;
};

class IRPCClientLocal : public IRPCClient{
public:
    void registerNode(const std::string& node_id, StorageEngine* storage_engine);
    bool sendPut(NodeInfo& node_info, std::string key, VersionedValue& value) override;
    std::optional<VersionedValue> sendGet(NodeInfo& node_info, std::string key) override;

private:
    std::map<std::string, StorageEngine*> node_registry_;
};