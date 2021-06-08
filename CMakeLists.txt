cmake_minimum_required (VERSION 2.8.11)
project (Buddy_Alloc)

add_library (bit-buddy BitBuddyAlloc.cpp)
target_include_directories (bit-buddy PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable (bit-buddy-test main.cpp)

target_link_libraries (bit-buddy-test LINK_PUBLIC bit-buddy)
