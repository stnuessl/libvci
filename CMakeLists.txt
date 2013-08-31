cmake_minimum_required(VERSION 2.6)

# project name
project(libvci C)

add_definitions(-Wall)
add_definitions(-O2)
#add_definitions(-g3)

find_package(Threads)

add_library(vci
	    SHARED
	    src/lib/clock/clock.c
	    src/lib/config/config.c
            src/lib/container/bintree.c
            src/lib/container/buffer.c
            src/lib/container/hash.c
            src/lib/container/heap.c
            src/lib/container/item.c
            src/lib/container/list.c
            src/lib/container/queue.c
            src/lib/container/stack.c
            src/lib/container/tree.c
            src/lib/etrace/etrace.c
            src/lib/mempool/mempool.c
            src/lib/threadpool/threadpool.c
            src/lib/threadpool/task.c
            )
            
target_link_libraries(vci ${CMAKE_THREAD_LIBS_INIT})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/)

install(FILES
        include/bintree.h
        include/buffer.h
        include/clock.h
        include/config.h
        include/etrace.h
        include/hash.h
        include/heap.h
        include/item.h
        include/list.h
        include/mempool.h
        include/queue.h
        include/stack.h
        include/task.h
        include/threadpool.h
        include/tree.h
        DESTINATION
        ${CMAKE_CURRENT_SOURCE_DIR}/build/include
        )