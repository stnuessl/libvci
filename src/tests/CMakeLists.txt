cmake_minimum_required(VERSION 2.6)

# project name
project(libvci_tests C)

add_definitions(-Wall)
add_definitions(-O2)
#add_definitions(-g3)

set(LIBS vci)
set(CMAKE_LIBRARY_PATH /usr/local/lib)

link_directories(${CMAKE_LIBRARY_PATH})

find_package(Threads)

add_executable(avltree_test container/avltree_test.c)
target_link_libraries(avltree_test ${LIBS})

add_executable(list_test container/list_test.c)
target_link_libraries(list_test ${LIBS})

add_executable(map_test container/map_test.c)
target_link_libraries(map_test ${LIBS})

add_executable(cache_test container/cache_test.c)
target_link_libraries(cache_test ${LIBS})

add_executable(heap_test container/heap_test.c)
target_link_libraries(heap_test ${LIBS})

add_executable(stack_test container/stack_test.c)
target_link_libraries(stack_test ${LIBS})

add_executable(queue_test container/queue_test.c)
target_link_libraries(queue_test ${LIBS})

add_executable(buffer_test container/buffer_test.c)
target_link_libraries(buffer_test ${LIBS})

add_executable(vector_test container/vector_test.c)
target_link_libraries(vector_test ${LIBS})

add_executable(threadpool_test concurrent/threadpool_test.c)
target_link_libraries(threadpool_test ${LIBS})

add_executable(clock_test util/clock_test.c)
target_link_libraries(clock_test ${LIBS})

add_executable(config_test util/config_test.c)
target_link_libraries(config_test ${LIBS})
target_link_libraries(config_test ${CMAKE_THREAD_LIBS_INIT})

add_executable(mempool_test util/mempool_test.c)
target_link_libraries(mempool_test ${LIBS})

add_executable(log_test util/log_test.c)
target_link_libraries(log_test ${LIBS})

add_executable(random_test util/random_test.c)
target_link_libraries(random_test ${LIBS})

add_executable(options_test util/options_test.c)
target_link_libraries(options_test ${LIBS})