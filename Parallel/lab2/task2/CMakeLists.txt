find_package(OpenMP REQUIRED COMPONENTS CXX)

add_executable(task2_omp main.cpp)

target_compile_features(task2_omp PRIVATE cxx_std_17)
target_link_libraries(task2_omp PRIVATE OpenMP::OpenMP_CXX)
