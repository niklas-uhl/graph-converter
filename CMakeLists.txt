cmake_minimum_required(VERSION 3.15)
project(graph_converter)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
find_package(TBB REQUIRED)
find_package(OpenMP)
message(STATUS ${CMAKE_MODULE_PATH})
find_package(Boost COMPONENTS iostreams REQUIRED )

include(FetchContent)
add_subdirectory(extern/ips4o)

FetchContent_Declare(
        CLI11
        GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
        GIT_TAG v1.9.0
)
FetchContent_MakeAvailable(CLI11)

FetchContent_Declare(
        stxxl
        GIT_REPOSITORY https://github.com/stxxl/stxxl.git
        GIT_TAG 1.4.1
)
FetchContent_MakeAvailable(stxxl)
message(STATUS ${stxxl_BINARY_DIR})
target_include_directories(stxxl SYSTEM PUBLIC  
    $<BUILD_INTERFACE:${stxxl_SOURCE_DIR}/include/> 
    $<BUILD_INTERFACE:${stxxl_BINARY_DIR}/include/> 
)


add_subdirectory(apps)
add_subdirectory(lib)

