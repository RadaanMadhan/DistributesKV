#include <gtest/gtest.h>
#include "../src/router/request_router.hpp"
#include <future>
#include <chrono>

// Timeout helper for tests (in milliseconds)
#define TEST_TIMEOUT_MS 5000

template<typename F>
void RunWithTimeout(F&& func, int timeout_ms = TEST_TIMEOUT_MS) {
    auto future = std::async(std::launch::async, std::forward<F>(func));
    auto status = future.wait_for(std::chrono::milliseconds(timeout_ms));
    ASSERT_NE(status, std::future_status::timeout) << "Test exceeded timeout of " << timeout_ms << "ms";
}

class RequestRouterTest : public ::testing::Test {
protected:
    std::vector<NodeInfo> CreateTestNodes(int count) {
        std::vector<NodeInfo> nodes;
        for (int i = 0; i < count; i++) {
            nodes.push_back(NodeInfo{
                "node" + std::to_string(i), 
                "127.0.0.1", 
                8000 + i
            });
        }
        return nodes;
    }
};

TEST_F(RequestRouterTest, ConstructorWithNodes) {
    RunWithTimeout([this]() {
        auto nodes = CreateTestNodes(3);
        EXPECT_NO_THROW(RequestRouter router(nodes, 150));
    });
}

TEST_F(RequestRouterTest, GetReplicasReturnsCorrectCount) {
    RunWithTimeout([this]() {
        auto nodes = CreateTestNodes(5);
        RequestRouter router(nodes, 150);
        
        auto replicas = router.GetReplicas("test_key", 3);
        EXPECT_EQ(replicas.size(), 3);
    });
}

TEST_F(RequestRouterTest, GetReplicasConsistentForSameKey) {
    RunWithTimeout([this]() {
        auto nodes = CreateTestNodes(5);
        RequestRouter router(nodes, 150);
        
        auto replicas1 = router.GetReplicas("test_key", 3);
        auto replicas2 = router.GetReplicas("test_key", 3);
        
        ASSERT_EQ(replicas1.size(), replicas2.size());
        for (size_t i = 0; i < replicas1.size(); i++) {
            EXPECT_EQ(replicas1[i].id_, replicas2[i].id_);
        }
    });
}

TEST_F(RequestRouterTest, GetReplicasDifferentForDifferentKeys) {
    RunWithTimeout([this]() {
        auto nodes = CreateTestNodes(5);
        RequestRouter router(nodes, 150);
        
        auto replicas1 = router.GetReplicas("key1", 3);
        auto replicas2 = router.GetReplicas("key2", 3);
        
        // Different keys should likely get different replica sets
        // (not guaranteed but highly probable with good hash distribution)
        bool different = false;
        for (size_t i = 0; i < replicas1.size(); i++) {
            if (replicas1[i].id_ != replicas2[i].id_) {
                different = true;
                break;
            }
        }
        EXPECT_TRUE(different);
    });
}
