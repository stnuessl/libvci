cmake_minimum_required(VERSION 2.6)

# project name
project(libvci C)

add_definitions(-D_GNU_SOURCE)
add_definitions(-Wall)
add_definitions(-Wextra)
add_definitions(-Wpedantic)
add_definitions(-O2)
#add_definitions(-g3)

set(CMAKE_C_FLAGS "-std=c99")
set(TARGET vci)
set(CMAKE_INSTALL_PREFIX /usr/local)
set(TARGET_SONAME lib${TARGET}.so)
set(TARGET_INSTALL_DIR lib)
set(HEADER_INSTALL_DIR include/libvci)

set(HEADER
    include/avltree.h
    include/buffer.h
    include/clock.h
    include/compare.h
    include/config.h
    include/error.h
    include/filesystem.h
    include/hash.h
    include/heap.h
    include/map.h
    include/link.h
    include/list.h
    include/log.h
    include/macro.h
    include/mempool.h
    include/options.h
    include/queue.h
    include/random.h
    include/stack.h
    include/threadpool.h
    include/vector.h
    )
        
set(SOURCE
    src/lib/concurrent/threadpool.c
    src/lib/container/avltree.c
    src/lib/container/buffer.c
    src/lib/container/container_p.c
    src/lib/container/heap.c
    src/lib/container/list.c
    src/lib/container/map.c
    src/lib/container/queue.c
    src/lib/container/stack.c
    src/lib/container/vector.c
    src/lib/util/clock.c
    src/lib/util/compare.c
    src/lib/util/config.c
    src/lib/util/config_parser.c
    src/lib/util/error.c
    src/lib/util/filesystem.c
    src/lib/util/hash.c
    src/lib/util/log.c
    src/lib/util/mempool.c
    src/lib/util/options.c
    src/lib/util/random.c
    )

find_package(Threads)

add_library(${TARGET}
	    SHARED
            ${SOURCE}
            )
            
target_link_libraries(${TARGET} ${CMAKE_THREAD_LIBS_INIT})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/)

install(FILES
        ${HEADER}
        DESTINATION
        ${HEADER_INSTALL_DIR}
        )
        
install(FILES
        ${CMAKE_BINARY_DIR}/${TARGET_SONAME}
        DESTINATION
        ${TARGET_INSTALL_DIR})