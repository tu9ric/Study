find_package(OpenMP REQUIRED COMPONENTS CXX)

add_executable(task2_omp main.cpp)

target_compile_features(task2_omp PRIVATE cxx_std_17)
target_link_libraries(task2_omp PRIVATE OpenMP::OpenMP_CXX)

# Keep source literals and output bytes in UTF-8 on both platforms.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(task2_omp PRIVATE -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()
