#include <gtest/gtest.h>
#include "../src/common/thread_pool.hpp"
#include <chrono>
#include <atomic>

class ThreadPoolTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }
};

TEST_F(ThreadPoolTest, ConstructorWithValidThreadCount) {
    EXPECT_NO_THROW(ThreadPool pool(4));
}

TEST_F(ThreadPoolTest, ConstructorWithZeroThreads) {
    EXPECT_NO_THROW(ThreadPool pool(0));
}

TEST_F(ThreadPoolTest, ConstructorWithNegativeThreadsThrows) {
    EXPECT_THROW(ThreadPool pool(-1), std::runtime_error);
}

TEST_F(ThreadPoolTest, EnqueueSimpleTask) {
    ThreadPool pool(2);
    std::atomic<int> counter(0);
    
    auto future = pool.enqueue([&counter]() {
        counter++;
        return 42;
    });
    
    EXPECT_EQ(future.get(), 42);
    EXPECT_EQ(counter.load(), 1);
}

TEST_F(ThreadPoolTest, EnqueueMultipleTasks) {
    ThreadPool pool(4);
    std::atomic<int> counter(0);
    std::vector<std::future<void>> futures;
    
    for (int i = 0; i < 10; i++) {
        futures.push_back(pool.enqueue([&counter]() {
            counter++;
        }));
    }
    
    for (auto& future : futures) {
        future.wait();
    }
    
    EXPECT_EQ(counter.load(), 10);
}

TEST_F(ThreadPoolTest, TasksExecuteInParallel) {
    ThreadPool pool(2);
    std::atomic<int> running(0);
    std::atomic<int> max_running(0);
    
    auto task = [&running, &max_running]() {
        running++;
        int current = running.load();
        if (current > max_running.load()) {
            max_running = current;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        running--;
    };
    
    std::vector<std::future<void>> futures;
    for (int i = 0; i < 4; i++) {
        futures.push_back(pool.enqueue(task));
    }
    
    for (auto& future : futures) {
        future.wait();
    }
    
    EXPECT_GE(max_running.load(), 2);
}
