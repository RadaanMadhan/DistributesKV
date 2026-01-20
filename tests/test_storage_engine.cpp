#include <gtest/gtest.h>
#include "../src/storage/storage_engine.hpp"

class StorageEngineTest : public ::testing::Test {
protected:
    StorageEngine storage;
};

TEST_F(StorageEngineTest, PutAndGet) {
    VersionedValue value{"test_value", 1, "node1"};
    storage.put("key1", value);
    
    auto result = storage.get("key1");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->value, "test_value");
    EXPECT_EQ(result->timestamp, 1);
}

TEST_F(StorageEngineTest, GetNonExistentKey) {
    auto result = storage.get("non_existent");
    EXPECT_FALSE(result.has_value());
}

TEST_F(StorageEngineTest, UpdateExistingKey) {
    VersionedValue value1{"value1", 1, "node1"};
    VersionedValue value2{"value2", 2, "node1"};
    
    storage.put("key1", value1);
    storage.put("key1", value2);
    
    auto result = storage.get("key1");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->value, "value2");
    EXPECT_EQ(result->timestamp, 2);
}

TEST_F(StorageEngineTest, MultipleKeys) {
    storage.put("key1", VersionedValue{"value1", 1, "node1"});
    storage.put("key2", VersionedValue{"value2", 1, "node1"});
    storage.put("key3", VersionedValue{"value3", 1, "node1"});
    
    auto result1 = storage.get("key1");
    auto result2 = storage.get("key2");
    auto result3 = storage.get("key3");
    
    ASSERT_TRUE(result1.has_value());
    ASSERT_TRUE(result2.has_value());
    ASSERT_TRUE(result3.has_value());
    
    EXPECT_EQ(result1->value, "value1");
    EXPECT_EQ(result2->value, "value2");
    EXPECT_EQ(result3->value, "value3");
}
