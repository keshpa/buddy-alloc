cmake_minimum_required (VERSION 2.8.11)
project (Buddy_Alloc)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")

add_executable (bit-buddy-test main.cpp)

target_link_libraries (bit-buddy-test LINK_PUBLIC)
