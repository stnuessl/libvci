cmake_minimum_required(VERSION 2.6)

# project name
project(libvci_tests C)

add_definitions(-DETRACE_DEBUG=2)
add_definitions(-Wall)
#add_definitions(-O2)
add_definitions(-g3)

find_package(Threads)

# library path
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../build/)

# project include path
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../build/include)

add_executable(list_test list_test.c)
target_link_libraries(list_test vci)

add_executable(map_test map_test.c)
target_link_libraries(map_test vci)

add_executable(stack_test stack_test.c)
target_link_libraries(stack_test vci)
# 
# add_executable(bintree_test bintree_test.c)
# target_link_libraries(bintree_test vci)
# 
# add_executable(heap_test heap_test.c)
# target_link_libraries(heap_test vci)
# 
# add_executable(buffer_test buffer_test.c)
# target_link_libraries(buffer_test vci)
# 
# add_executable(threadpool_test threadpool_test.c)
# target_link_libraries(threadpool_test vci)
# 
# add_executable(clock_test clock_test.c)
# target_link_libraries(clock_test vci)
# 
# add_executable(config_test config_test.c)
# target_link_libraries(config_test vci)
# target_link_libraries(config_test ${CMAKE_THREAD_LIBS_INIT})
# 
# add_executable(etrace_test etrace_test.c)
# target_link_libraries(etrace_test vci)

add_executable(mempool_test mempool_test.c)
target_link_libraries(mempool_test vci)

# add_executable(log_test log_test.c)
# target_link_libraries(log_test vci)
# 
# add_executable(allocator_test allocator_test.c)
# target_link_libraries(allocator_test vci)
# 
# add_executable(vector_test vector_test.c)
# target_link_libraries(vector_test vci)