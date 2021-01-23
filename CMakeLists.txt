cmake_minimum_required(VERSION 3.17)
project(graph_converter)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenMP)

include(FetchContent)
FetchContent_Declare(
        ips4o
        GIT_REPOSITORY https://github.com/ips4o/ips4o.git
)
FetchContent_MakeAvailable(ips4o)
add_subdirectory(apps)
add_subdirectory(lib)

