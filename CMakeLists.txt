cmake_minimum_required(VERSION 3.28)
project(mb_grep)

set(CMAKE_CXX_STANDARD 20)

add_executable(mb_grep
        main.cpp
        matcher.h
        matcher.cpp
        utils.h
        utils.cpp
        thread_pool.h
        thread_pool.cpp)
if (UNIX OR APPLE)
    target_link_libraries(mb_grep pthread)
endif ()
