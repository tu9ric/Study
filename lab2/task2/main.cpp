#include "../include/console.hpp"
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <iomanip>
#include <string>
#include <cmath>
#include <algorithm>
#include <omp.h>

#ifndef LABS_MATRIX_SIZE
#define LABS_MATRIX_SIZE 2048
#endif
#ifndef LABS_REPEATS
#define LABS_REPEATS 10
#endif
constexpr std::size_t N = LABS_MATRIX_SIZE;
constexpr std::size_t THREAD_COUNT = 14;
constexpr std::size_t NUM_OF_TESTS = LABS_REPEATS;

/// @brief основная структура теста
struct Test
{
    std::size_t id;
    double timeLinear;
    double timeParallel;
    bool comparison;
};

/// @brief функция заполнения матрицы случайными double числаами в диапазоне от 0.0 до 10.0 
/// @param matrix матрица, которую необходимо заполнить случайными числами
void fillMatrix(double (&matrix)[N][N], std::mt19937& gen)
{
    std::uniform_real_distribution<double> dist(0.0, 10.0);

    for(std::size_t i = 0; i < N; i++)
    {
        for(std::size_t j = 0; j < N; j++)
        {
            matrix[i][j] = dist(gen);
        }
    }
}

/// @brief функция для перемножения матриц
/// @param a первая матрица
/// @param b вторая матрица
/// @param res произведение двух матриц
/// @param startRow строка с которой начинает конкретный поток считать (по умолчанию 0)
/// @param endRow строка до которой считает поток (по умолчанию N)
void multiplyMatricesSequential(
    const double (&a)[N][N], 
    const double (&b)[N][N], 
    double (&res)[N][N]
)
{
    for(std::size_t i = 0; i < N; ++i)
    {
        for(std::size_t j = 0; j < N; ++j)
        {
            res[i][j] = 0.0;
            // подсчёт суммы в строке матрицы
            for(std::size_t k = 0; k < N; k++)
            {
                res[i][j] += a[i][k] * b[k][j];
            }
        }
    }
}

void multiplyMatricesParallel(
    const double (&a)[N][N],
    const double (&b)[N][N],
    double (&res)[N][N])
{
    #pragma omp parallel for schedule(static)
    for(std::size_t i = 0; i < N; ++i)
    {
        for(std::size_t j = 0; j < N; ++j)
        {
            res[i][j] = 0.0;
            
            for(std::size_t k = 0; k < N; ++k)
            {
                res[i][j] += a[i][k] * b[k][j];
            }
        }
    }
}

/// @brief функция для сравнения двух матриц
/// @param a первая матрица
/// @param b вторая матрица
/// @return 1 - если матрицы идентичны, 0 - если в матрицах есть различия
bool compareResults(
    const double (&a)[N][N], 
    const double (&b)[N][N])
{
    constexpr double ABS_EPS = 1e-9;
    constexpr double REL_EPS = 1e-12;

    for(std::size_t i = 0; i < N; i++)
    {
        for(std::size_t j = 0; j < N; j++)
        {
            const double difference = std::abs(a[i][j] - b[i][j]);
            const double tolerance = ABS_EPS + REL_EPS * std::max(std::abs(a[i][j]), std::abs(b[i][j]));

            if(!std::isfinite(a[i][j]) || 
               !std::isfinite(b[i][j]) || 
               difference > tolerance)
            {
                std::cerr << "Mismatch at [" << i << "][" << j << "]: " << std::setprecision(17) << a[i][j] << " vs " << b[i][j] << '\n';
                return false;
            }
        }
    }
    return true;
}

/// @brief функция для вывода матрицы в консоль
/// @param matrix матрица, которую необходимо вывести
void printMatrix(double (&matrix)[N][N])
{
    std::cout << '\n';
    for(std::size_t i = 0; i < N; i++)
    {
        for(std::size_t j = 0; j < N; j++)
        {
            std::cout << matrix[i][j] << "\t";
        }
        std::cout << "\n";
    }
    std::cout << '\n';
}


/// @brief запуск одного теста
/// @param id айди теста
/// @param a 
/// @param b 
/// @param resLinear 
/// @param resParallel 
/// @return 
Test oneTest(
    std::size_t id,
    const double (&a)[N][N],
    const double (&b)[N][N],
    double (&resLinear)[N][N],
    double (&resParallel)[N][N])
{
    Test test{};
    test.id = id;

    auto measureSequential = [&]()
    {
        const auto start = std::chrono::steady_clock::now();

        multiplyMatricesSequential(a, b, resLinear);

        const auto end = std::chrono::steady_clock::now();
        test.timeLinear =
            std::chrono::duration<double>(end - start).count();
    };

    auto measureParallel = [&]()
    {
        const auto start = std::chrono::steady_clock::now();

        multiplyMatricesParallel(a, b, resParallel);

        const auto end = std::chrono::steady_clock::now();
        test.timeParallel =
            std::chrono::duration<double>(end - start).count();
    };

    // Чередуем порядок запусков.
    if (id % 2 != 0)
    {
        measureSequential();
        measureParallel();
    }
    else
    {
        measureParallel();
        measureSequential();
    }

    // Проверка не входит в замеры времени.
    test.comparison = compareResults(resLinear, resParallel);

    return test;
}

void printResult(const std::vector<Test>& tests)
{
    std::cout << std::setw(5) << "ID   "
              << std::setw(20) << "LinearTime (s)"
              << std::setw(20) << "ParallelTime (s)"
              << std::setw(15) << "Comparison" << "\n";

    std::cout << std::string(60, '-') << "\n";

    for (const auto& test : tests)
    {
        std::cout << std::setw(5) << test.id
                  << std::setw(20) << test.timeLinear
                  << std::setw(20) << test.timeParallel
                  << std::setw(15) << test.comparison << "\n";
    }
}

int main()
{
    initializeConsoleUtf8();
    omp_set_dynamic(0);
    omp_set_num_threads(THREAD_COUNT);

    static double a[N][N];
    static double b[N][N];
    static double resLinear[N][N];
    static double resParallel[N][N];

    std::mt19937 gen(42);
    fillMatrix(a, gen);
    fillMatrix(b, gen);

    std::vector<Test> tests;
    tests.reserve(NUM_OF_TESTS);

    for (std::size_t i = 0; i < NUM_OF_TESTS; ++i)
    {
        tests.push_back(
            oneTest(i + 1, a, b, resLinear, resParallel));
    }

    printResult(tests);

    return std::all_of(
        tests.begin(), tests.end(),
        [](const Test& test) { return test.comparison; }) ? 0 : 1;
}