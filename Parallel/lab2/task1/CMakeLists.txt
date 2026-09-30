# core нужен для матрицы изображения
# imgcodecs нужен для чтения и записи файлов
# highgui нужен для создания окна
find_package(OpenCV REQUIRED COMPONENTS core imgcodecs highgui)
find_package(OpenMP REQUIRED COMPONENTS CXX)

add_executable(lab2_seq main.cpp)
add_executable(lab2_omp main.cpp)

foreach(target IN ITEMS lab2_seq lab2_omp)
    target_compile_features(${target} PRIVATE cxx_std_17)
    target_compile_definitions(${target} PRIVATE
        TASK1_OUTPUT_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
    )
    target_include_directories(${target} PRIVATE ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(${target} PRIVATE ${OpenCV_LIBS})
endforeach()

target_link_libraries(lab2_omp PRIVATE OpenMP::OpenMP_CXX)
