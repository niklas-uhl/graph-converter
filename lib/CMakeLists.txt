add_library(converter-lib INTERFACE)
target_include_directories(converter-lib INTERFACE ${PROJECT_SOURCE_DIR}/lib)
target_compile_features(converter-lib INTERFACE cxx_std_17)
