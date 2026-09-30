# core нужен для матрицы изображения
# imgcodecs нужен для чтения и записи файлов
# highgui нужен для создания окна
find_package(OpenCV REQUIRED COMPONENTS core imgcodecs)
find_package(OpenMP REQUIRED COMPONENTS CXX)

add_executable(lab2_seq main.cpp)
add_executable(lab2_omp main.cpp)

foreach(target IN ITEMS lab2_seq lab2_omp)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${target}-output")
    target_compile_features(${target} PRIVATE cxx_std_17)
    target_compile_definitions(${target} PRIVATE
        TASK1_OUTPUT_DIR="${CMAKE_CURRENT_BINARY_DIR}/${target}-output"
    )
    target_include_directories(${target} PRIVATE ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(${target} PRIVATE ${OpenCV_LIBS})
endforeach()

target_link_libraries(lab2_omp PRIVATE OpenMP::OpenMP_CXX)

# Keep source literals and output bytes in UTF-8 on both platforms.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(lab2_seq PRIVATE -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(lab2_omp PRIVATE -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()
