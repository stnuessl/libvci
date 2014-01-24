cmake_minimum_required(VERSION 2.6)

# project name
project(libvci_tests C)

add_definitions(-DETRACE_DEBUG=2)
add_definitions(-Wall)
add_definitions(-O2)
#add_definitions(-g3)

find_package(Threads)

# library path
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../build/)

# project include path
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../build/include)

add_executable(avltree_test container/avltree_test.c)
target_link_libraries(avltree_test vci)

add_executable(list_test container/list_test.c)
target_link_libraries(list_test vci)

add_executable(map_test container/map_test.c)
target_link_libraries(map_test vci)

add_executable(cache_test container/cache_test.c)
target_link_libraries(cache_test vci)

add_executable(stack_test container/stack_test.c)
target_link_libraries(stack_test vci)

add_executable(queue_test container/queue_test.c)
target_link_libraries(queue_test vci)

add_executable(buffer_test container/buffer_test.c)
target_link_libraries(buffer_test vci)

add_executable(ringbuffer_test container/ringbuffer_test.c)
target_link_libraries(ringbuffer_test vci)
target_link_libraries(ringbuffer_test ${CMAKE_THREAD_LIBS_INIT})

add_executable(threadpool_test threadpool/threadpool_test.c)
target_link_libraries(threadpool_test vci)

add_executable(clock_test clock/clock_test.c)
target_link_libraries(clock_test vci)

add_executable(config_test config/config_test.c)
target_link_libraries(config_test vci)
target_link_libraries(config_test ${CMAKE_THREAD_LIBS_INIT})

add_executable(etrace_test etrace/etrace_test.c)
target_link_libraries(etrace_test vci)

add_executable(mempool_test mempool/mempool_test.c)
target_link_libraries(mempool_test vci)

add_executable(log_test log/log_test.c)
target_link_libraries(log_test vci)

add_executable(vector_test container/vector_test.c)
target_link_libraries(vector_test vci)

add_executable(observer_test observer/observer_test.c)
target_link_libraries(observer_test vci)