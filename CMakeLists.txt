cmake_minimum_required(VERSION 3.26)
project(SignalCluster)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -g -pthread ")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/product/lib) # 动态库 .so/.dylib/.dll
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/product/bin) # 可执行文件

add_subdirectory(proto)

add_subdirectory(test)

