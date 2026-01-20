#include <gtest/gtest.h>
#include "../src/router/request_router.hpp"

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
    auto nodes = CreateTestNodes(3);
    EXPECT_NO_THROW(RequestRouter router(nodes, 150));
}

TEST_F(RequestRouterTest, GetReplicasReturnsCorrectCount) {
    auto nodes = CreateTestNodes(5);
    RequestRouter router(nodes, 150);
    
    auto replicas = router.GetReplicas("test_key", 3);
    EXPECT_EQ(replicas.size(), 3);
}

TEST_F(RequestRouterTest, GetReplicasConsistentForSameKey) {
    auto nodes = CreateTestNodes(5);
    RequestRouter router(nodes, 150);
    
    auto replicas1 = router.GetReplicas("test_key", 3);
    auto replicas2 = router.GetReplicas("test_key", 3);
    
    ASSERT_EQ(replicas1.size(), replicas2.size());
    for (size_t i = 0; i < replicas1.size(); i++) {
        EXPECT_EQ(replicas1[i].id, replicas2[i].id);
    }
}

TEST_F(RequestRouterTest, DifferentKeysGetDifferentReplicas) {
    auto nodes = CreateTestNodes(5);
    RequestRouter router(nodes, 150);
    
    auto replicas1 = router.GetReplicas("key1", 3);
    auto replicas2 = router.GetReplicas("key2", 3);
    
    // Different keys should likely get different replica sets
    // (not guaranteed but highly probable with good hash distribution)
    bool different = false;
    for (size_t i = 0; i < replicas1.size(); i++) {
        if (replicas1[i].id != replicas2[i].id) {
            different = true;
            break;
        }
    }
    EXPECT_TRUE(different);
}
