cmake_minimum_required(VERSION 3.10)
project(MyTests)

enable_testing()

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/src)

# Collect all source files from src directories
set(COMMON_SOURCES
    ../src/common/thread_pool.cpp
)

set(STORAGE_SOURCES
    ../src/storage/storage_engine.cpp
)

set(ROUTER_SOURCES
    ../src/router/request_router.cpp
)

set(REPLICATION_SOURCES
    ../src/replication/replication_coordinator.cpp
)

set(IRPC_SOURCES
    ../src/replication/irpc.cpp
)

set(API_SOURCES
    ../src/api/api.cpp
)

# Test executables
add_executable(test_hello test_hello.cpp)
target_link_libraries(test_hello ${GTEST_LIBRARIES} gtest_main pthread)
add_test(NAME test_hello COMMAND test_hello)

add_executable(test_thread_pool test_thread_pool.cpp ${COMMON_SOURCES})
target_link_libraries(test_thread_pool ${GTEST_LIBRARIES} gtest_main pthread)
add_test(NAME test_thread_pool COMMAND test_thread_pool)

add_executable(test_storage_engine test_storage_engine.cpp ${STORAGE_SOURCES})
target_link_libraries(test_storage_engine ${GTEST_LIBRARIES} gtest_main pthread)
add_test(NAME test_storage_engine COMMAND test_storage_engine)

add_executable(test_request_router test_request_router.cpp ${ROUTER_SOURCES})
target_link_libraries(test_request_router ${GTEST_LIBRARIES} gtest_main pthread)
add_test(NAME test_request_router COMMAND test_request_router)

add_executable(test_replication_coordinator test_replication_coordinator.cpp ${REPLICATION_SOURCES} ${ROUTER_SOURCES} ${COMMON_SOURCES} ${IRPC_SOURCES} ${STORAGE_SOURCES})
target_link_libraries(test_replication_coordinator ${GTEST_LIBRARIES} gtest_main pthread)
add_test(NAME test_replication_coordinator COMMAND test_replication_coordinator)

add_executable(test_api_server test_api_server.cpp ${API_SOURCES} ${IRPC_SOURCES} ${REPLICATION_SOURCES} ${ROUTER_SOURCES} ${COMMON_SOURCES} ${STORAGE_SOURCES})
target_link_libraries(test_api_server ${GTEST_LIBRARIES} gtest_main pthread)
add_test(NAME test_api_server COMMAND test_api_server)