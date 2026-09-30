#include "../common/console.hpp"
#include <cstdint>
#include <algorithm>
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <array>
#include <atomic>
#include <thread>
#include <iomanip>

#ifndef LABS_MATRIX_SIZE
#define LABS_MATRIX_SIZE 2048
#endif
#ifndef LABS_REPEATS
#define LABS_REPEATS 10
#endif
constexpr std::size_t N = LABS_MATRIX_SIZE;
constexpr std::size_t THREAD_COUNT = 14;
constexpr std::size_t ROWSPERTHREAD = N / THREAD_COUNT;
constexpr std::size_t NUM_OF_TESTS = LABS_REPEATS;

/// @brief генерация уникальных id
class IDGenerator 
{
private: 
    // Статический атомарный счётчик
    inline static std::atomic<uint64_t> next_id{1};

public: 
    static uint64_t generate()
    {
        return next_id.fetch_add(1, std::memory_order_relaxed);
    }
};

/// @brief основная структура теста
struct Test
{
    std::size_t id;
    double timeLinear;
    double timeParallel;
    bool comparison;
};


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

/// @brief функция заполнения матрицы случайными double числаами в диапазоне от 0.0 до 10.0 
/// @param matrix матрица, которую необходимо заполнить случайными числами
void fillMatrix(double (&matrix)[N][N])
{
    std::random_device rd;
    std::mt19937 gen(rd());
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
void multiplyMatrices(
    const double (&a)[N][N], 
    const double (&b)[N][N], 
    double (&res)[N][N],
    std::size_t startRow = 0,
    std::size_t endRow = N)
{
    for(std::size_t i = startRow; i < endRow; i++)
    {
        for(std::size_t j = 0; j < N; j++)
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

/// @brief функция для сравнения двух матриц
/// @param a первая матрица
/// @param b вторая матрица
/// @return 1 - если матрицы идентичны, 0 - если в матрицах есть различия
bool compareResults(const double (&a)[N][N], const double (&b)[N][N])
{
    for(std::size_t i = 0; i < N; i++)
    {
        for(std::size_t j = 0; j < N; j++)
        {
            if(a[i][j] != b[i][j])
            {
                return 0;
            }
        }
    }
    return 1;
}

/// @brief функция запуска одного теста
/// @param a первая матрица
/// @param b вторая матрица
/// @param resLinear результат перемножения матриц линейно
/// @param resParallel результат перемножения матриц параллельно
/// @return объект структуры Test в котором хранится время выполнения линейного и распараллеленных вычислений
Test oneTest(
    const double (&a)[N][N], 
    const double (&b)[N][N], 
    double (&resLinear)[N][N],
    double (&resParallel)[N][N]
)
{
    Test test;
    test.id = IDGenerator::generate();
    auto start = std::chrono::steady_clock::now();
    // умножение матриц
    multiplyMatrices(a, b, resLinear);
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    double TimeLinear = elapsed.count();
    test.timeLinear = TimeLinear;


    //многопоточный замер
    start = std::chrono::steady_clock::now();
    std::size_t baseRows = N / THREAD_COUNT;
    std::size_t remainder = N % THREAD_COUNT;

    // массив со всеми потоками
    std::array<std::thread, THREAD_COUNT> threads;

    std::size_t startRow = 0;

    for(std::size_t t = 0; t < THREAD_COUNT; ++t)
    {
        // раскидываю остаток от деления между потоками для равномерного распределения нагрузки
        std::size_t rowsForThread = baseRows + (t < remainder ? 1 : 0);

        std::size_t endRow = startRow + rowsForThread;

        threads[t] = std::thread([&, startRow, endRow]()
        {
            multiplyMatrices(a, b, resParallel, startRow, endRow);
        });

        startRow = endRow;
    }

    // Завершаем работу всех потоков
    for(auto& thread : threads)
    {
        // join() блокирует вызывающий поток до завершения соответствующего рабочего потока
        thread.join();
    }

    end = std::chrono::steady_clock::now();
    elapsed = end - start;

    double TimeParallel = elapsed.count();
    test.timeParallel = TimeParallel;

    test.comparison = compareResults(resLinear, resParallel);

    return test;
}

void printResult(const std::vector<Test>& tests)
{
    for(auto& test : tests)
    {
        // Вывод строк таблицы
        std::cout << std::setw(5) << test.id
                  << std::setw(20) << test.timeLinear
                  << std::setw(20) << test.timeParallel
                  << std::setw(15) << test.comparison << "\n";
    }     
}

int main()
{
    initializeConsoleUtf8();
    // определение и заполнение матриц
    static double a[N][N];
    fillMatrix(a);

    static double b[N][N];
    fillMatrix(b);

    // инициализация матрицы для результата
    static double resLinear[N][N]{0};
    static double resParallel[N][N]{0};

    std::vector<Test> tests{};
    
    for(std::size_t i = 0; i < NUM_OF_TESTS; i++)
    {
        Test test = oneTest(a, b, resLinear, resParallel);
        tests.push_back(test);
    }   

    // Вывод заголовка
    std::cout << std::setw(5) << "ID   " 
              << std::setw(20) << "LinearTime" 
              << std::setw(20) << "ParallelTime"
              << std::setw(15) << "Comparison" << "\n";

    // Линия-разделитель
    std::cout << std::string(60, '-') << "\n";

    printResult(tests);


    return std::all_of(tests.begin(), tests.end(),
        [](const Test& test) { return test.comparison; }) ? 0 : 1;
}
