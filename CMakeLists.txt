cmake_minimum_required(VERSION 3.28)
project(mb_grep)

set(CMAKE_CXX_STANDARD 20)

#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")

add_executable(mb_grep main.cpp)
target_link_libraries(mb_grep pthread)
