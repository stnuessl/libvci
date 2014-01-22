cmake_minimum_required(VERSION 2.6)

# project name
project(libvci C)

add_definitions(-Wall)
#add_definitions(-O2)
add_definitions(-g3)

find_package(Threads)

add_library(vci
	    SHARED
	    src/lib/clock/clock.c
	    src/lib/config/config.c
	    src/lib/config/config_parser.c
	    src/lib/config/key.c
	    src/lib/container/avltree.c
            src/lib/container/buffer.c
            src/lib/container/cache.c
            src/lib/container/container_util.c
            src/lib/container/map.c
            src/lib/container/list.c
            src/lib/container/queue.c
            src/lib/container/stack.c
            src/lib/container/ringbuffer.c
            src/lib/container/vector.c
            src/lib/log/log.c
            src/lib/etrace/etrace.c
            src/lib/mempool/mempool.c
            src/lib/observer/subject.c
            src/lib/observer/observer.c
            src/lib/threadpool/threadpool.c
            src/lib/threadpool/task.c
            )
            
target_link_libraries(vci ${CMAKE_THREAD_LIBS_INIT})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/)

install(FILES
        include/avltree.h
        include/buffer.h
        include/cache.h
        include/clock.h
        include/config.h
        include/etrace.h
        include/map.h
        include/link.h
        include/list.h
        include/log.h
        include/macros.h
        include/mempool.h
        include/queue.h
        include/stack.h
        include/ringbuffer.h
        include/task.h
        include/threadpool.h
        include/vector.h
        include/observer.h
        include/subject.h
        DESTINATION
        ${CMAKE_CURRENT_SOURCE_DIR}/build/include
        )