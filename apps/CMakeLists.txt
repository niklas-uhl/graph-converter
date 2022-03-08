add_executable(metis2binary metis2binary.cpp)

target_link_libraries(metis2binary converter-lib)
if(OpenMP_CXX_FOUND)
    target_link_libraries(metis2binary OpenMP::OpenMP_CXX)
endif()
target_compile_features(metis2binary PUBLIC cxx_std_17)

add_executable(edgelist2metis edgelist2metis.cpp)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_link_libraries(edgelist2metis converter-lib ips4o CLI11::CLI11)
target_compile_definitions(edgelist2metis PRIVATE _REENTRANT)
target_compile_features(edgelist2metis PUBLIC cxx_std_17)

add_executable(reorder reorder.cpp)
target_link_libraries(reorder PRIVATE converter-lib CLI11::CLI11)
target_compile_features(reorder PUBLIC cxx_std_17)

add_executable(apply-partition apply-partition.cpp)
target_link_libraries(apply-partition PRIVATE converter-lib CLI11::CLI11)
target_compile_features(apply-partition PUBLIC cxx_std_17)

add_executable(graph-info graph-info.cpp)
target_link_libraries(graph-info converter-lib CLI11::CLI11)
if(OpenMP_CXX_FOUND)
    target_link_libraries(graph-info OpenMP::OpenMP_CXX)
endif()
target_compile_features(graph-info PUBLIC cxx_std_17)

add_executable(frequency-plot frequency-plot.cpp)
target_link_libraries(frequency-plot converter-lib CLI11::CLI11)
if(OpenMP_CXX_FOUND)
    target_link_libraries(frequency-plot OpenMP::OpenMP_CXX)
endif()
target_compile_features(frequency-plot PUBLIC cxx_std_17)
