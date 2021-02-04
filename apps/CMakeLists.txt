add_executable(metis2binary metis2binary.cpp)

target_link_libraries(metis2binary converter-lib)
if(OpenMP_CXX_FOUND)
    target_link_libraries(metis2binary OpenMP::OpenMP_CXX)
endif()

add_executable(edgelist2metis edgelist2metis.cpp)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_link_libraries(edgelist2metis converter-lib ips4o)
target_compile_definitions(edgelist2metis PRIVATE _REENTRANT)