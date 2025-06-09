cmake_minimum_required(VERSION 3.28)
project(mb_grep)

set(CMAKE_CXX_STANDARD 20)

#option(ENABLE_TSAN "Enable Thread Sanitizer" OFF)
#
#if(ENABLE_TSAN)
#    message(STATUS "Thread Sanitizer enabled")
#    add_compile_options(-fsanitize=thread -g)
#    add_link_options(-fsanitize=thread)
#endif()

add_executable(mb_grep main.cpp)
target_link_libraries(mb_grep pthread)
