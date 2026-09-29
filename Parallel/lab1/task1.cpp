#include <iostream>
#include <thread>
#include <chrono>
#include <array>
#include <iomanip>
#include <vector>
#include <string>
#include <algorithm>
#include <cstdint>
#include <atomic>
// библиотека для работы с SIMD в C++
#include <immintrin.h>

constexpr int NUM_OF_THREADS = 14;
constexpr std::size_t NUM_OF_TESTS = 10;

using Sum = __int128;

// генерация id теста
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

/// @brief перечисление всевозможных вариантов запуска тестов
enum class TypeOfTest
{
    Linear,
    Simd,
    Threads,
    ThreadsAndSimd, 
    Unknown
};

/// @brief Перевод типа теста в строку, чтобы вывести в консоль
/// @param type тип теста 
/// @return тип теста в строковом значении
const char* toString(TypeOfTest type)
{
    switch(type)
    {
        case TypeOfTest::Linear:
            return "Linear";
        case TypeOfTest::Simd:
            return "SIMD";
        case TypeOfTest::Threads:
            return "Threads";
        case TypeOfTest::ThreadsAndSimd:
            return "ThreadsAndSimd";
        default:
            return "Unknown";
    }
}

/// @brief основная структура теста
struct Test
{
    std::size_t id;
    TypeOfTest type{TypeOfTest::Unknown};
    double time;
    __int128 result;
};

/// @brief функция для сложения чисел 
/// @param begin первое значение диапазона
/// @param end первая граница диапазона
/// @return сумма чисел, находящихся в отрезке между begin и end
Sum sumOfRange(long long begin, long long end)
{
    Sum sum {0};
    for(long long i = begin; i <= end; ++i)
    {
        sum += static_cast<Sum>(i);
    }
    return sum;
}

// будем использовать 256-битные целочисленные AVX2 регистры __m256i
/// @brief сумма ряда через simd инструкции
/// @param begin 
/// @param end 
/// @return 
Sum sumOfRangeSIMD(const long long begin, const long long end)
{
    if(begin > end)
    {
        return 0;
    }

    constexpr long long BLOCK_SIZE = 1000000;

    Sum total{0};

    long long i = begin;

    const __m256i step = _mm256_set1_epi64x(4);

    while(i <= end)
    {
        const long long blockSize = std::min(BLOCK_SIZE, end - i + 1);

        const long long blockEnd = i + blockSize;

        __m256i sumSIMD = _mm256_setzero_si256();
        __m256i valuesSIMD = _mm256_setr_epi64x(i, i + 1, i + 2, i + 3);

        for(; blockEnd - i >= 4; i += 4)
        {
            sumSIMD = _mm256_add_epi64(sumSIMD, valuesSIMD);
            valuesSIMD = _mm256_add_epi64(valuesSIMD, step);
        }

        long long partialSums[4];
        _mm256_storeu_si256(
            reinterpret_cast<__m256i*>(partialSums), sumSIMD
        );

        for(long long value : partialSums)
        {
            total += static_cast<Sum>(value);
        }

        for(; i < blockEnd; ++i)
        {
            total += static_cast<Sum>(i);
        }
    }
    
    return total;
} 



Test LinearTest(Sum n)
{
    Test LinearTest{};
    auto start = std::chrono::steady_clock::now();
    Sum res = sumOfRange(0, n);
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> LinearTime = end - start;
    LinearTest.id = IDGenerator::generate();
    LinearTest.type = TypeOfTest::Linear;
    LinearTest.time = LinearTime.count();
    LinearTest.result = res;
    return LinearTest;
}

Test SimdTest(Sum n)
{
    Test SIMDTest{};
    auto start = std::chrono::steady_clock::now();
    Sum res = sumOfRangeSIMD(0, n);
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> SIMDTime = end - start;
    SIMDTest.id = IDGenerator::generate();
    SIMDTest.type = TypeOfTest::Simd;
    SIMDTest.time = SIMDTime.count();
    SIMDTest.result = res;
    return SIMDTest;
}

Test ThreadTest(long long n)
{
    Test THREADTest{};
    auto start = std::chrono::steady_clock::now();
    // массив для разделения одной большой задачи на части
    std::array<Sum, NUM_OF_THREADS> partial{};
    // массив со всеми потоками
    std::array<std::thread, NUM_OF_THREADS> threads;

    // цикл в котором мы присваиваем каждому потоку свою область действия
    // тут мы определяем границы каждой из областей
    for (std::size_t t = 0; t < NUM_OF_THREADS; ++t)
    {
        const long long threadIndex = static_cast<long long>(t);
        const long long count = n + 1;
        // просчитываем начальное значение области
        const Sum begin = count * threadIndex / NUM_OF_THREADS;
        // и конечное значение области
        const Sum end = count * (threadIndex + 1) / NUM_OF_THREADS - 1;
        
        // здесь мы определяем каждый из потоков 
        // используется лямбда выражение - поток начинает выполнять функцию сразу после создания 
        // в квадратных скобках пишутся захваты, чтобы использовать локальные переменные внутри лямбды
        threads[t] = std::thread([&partial, t, begin, end]()
        {
            // результат вычисляется для диапазона и записывается в partial
            partial[t] = sumOfRange(begin, end);
        });
    }

    // Завершаем работу всех потоков
    for(auto& thread : threads)
    {
        // join() блокирует вызывающий поток до завершения соответствующего рабочего потока
        thread.join();
    }

    Sum parallelRes = 0;
    for (Sum value : partial)
    {
        parallelRes += value;
    }

    auto endtime = std::chrono::steady_clock::now();
    std::chrono::duration<double> threadsTime = endtime - start;

    THREADTest.id = IDGenerator::generate();
    THREADTest.type = TypeOfTest::Threads;
    THREADTest.time = threadsTime.count();
    THREADTest.result = parallelRes;

    return THREADTest;
}

Test ThreadsAndSimdTest(long long n)
{
    Test THREADSANDSIMDTest;
    // ЗАМЕР РАСПАРАЛЛЕЛИВАНИЯ (12 ПОТОКОВ) + SIMD
    auto start = std::chrono::steady_clock::now();
    // массив для разделения одной большой задачи на части
    std::array<Sum, NUM_OF_THREADS> partial{};
    // массив со всеми потоками
    std::array<std::thread, NUM_OF_THREADS> threads;

    // цикл в котором мы присваиваем каждому потоку свою область действия
    // тут мы определяем границы каждой из областей
    for (std::size_t t = 0; t < NUM_OF_THREADS; ++t)
    {
        // просчитываем начальное значение области
        const __int128 begin = (n + 1) * t / NUM_OF_THREADS;
        // и конечное значение области
        const __int128 end = (n + 1) * (t + 1) / NUM_OF_THREADS - 1;
        
        // здесь мы определяем каждый из потоков 
        // используется лямбда выражение - поток начинает выполнять функцию сразу после создания 
        // в квадратных скобках пишутся захваты, чтобы использовать локальные переменные внутри лямбды
        threads[t] = std::thread([&partial, t, begin, end]()
        {
            // результат вычисляется для диапазона и записывается в partial
            partial[t] = sumOfRangeSIMD(begin, end);
        });
    }

    // Завершаем работу всех потоков
    for(auto& thread : threads)
    {
        // join() блокирует вызывающий поток до завершения соответствующего рабочего потока
        thread.join();
    }

    Sum parallelRes = 0;
    for (Sum value : partial)
    {
        parallelRes += value;
    }

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> threadsAndSIMDTime = end - start;

    THREADSANDSIMDTest.id = IDGenerator::generate();
    THREADSANDSIMDTest.type = TypeOfTest::ThreadsAndSimd;
    THREADSANDSIMDTest.time = threadsAndSIMDTime.count();
    THREADSANDSIMDTest.result = parallelRes;

    return THREADSANDSIMDTest;
}

std::string sumToString(Sum value)
{
    if(value == 0)
    {
        return "0";
    }

    std::string result;

    while(value > 0)
    {
        const int digit = static_cast<int>(value % 10);
        result.push_back(static_cast<char>('0' + digit));
        value /= 10;
    }

    std::reverse(result.begin(), result.end());
    return result;
}

void printResult(const std::vector<Test>& tests)
{
    for(auto& test : tests)
    {
        // Вывод строк таблицы
        std::cout << std::setw(10) << test.id
                  << std::setw(20) << toString(test.type)
                  << std::setw(20) << test.time 
                  << std::setw(30) << sumToString(test.result) << "\n";
    }     
}

int main()
{

    constexpr long long MAX_N = 10000000000LL;

    // добавил std::cin, чтобы были корректные замеры с -O2 оптимизацией
    // иначе компилятор заранее знает границы и выдаёт ответ сразу, что ломает эксперимент
    long long n;
    std::cout << "Enter n: ";
    
    if(!(std::cin >> n) || n < 0 || n > MAX_N)
    {
        std::cerr << "N must be an integer from 0 to " << MAX_N << '\n';
        return 1;
    }

    std::vector<Test> tests{};
    
    for(std::size_t i = 0; i < NUM_OF_TESTS; i++)
    {
        tests.push_back(LinearTest(n));
    }
    for(std::size_t i = 0; i < NUM_OF_TESTS; i++)
    {
        tests.push_back(SimdTest(n));
    }
    for(std::size_t i = 0; i < NUM_OF_TESTS; i++)
    {
        tests.push_back(ThreadTest(n));
    }
    for(std::size_t i = 0; i < NUM_OF_TESTS; i++)
    {
        tests.push_back(ThreadsAndSimdTest(n));
    }

    const Sum wideN = static_cast<Sum>(n);
    const Sum expected = wideN * (wideN + 1) / 2;

    for(const auto& test : tests)
    {
        if(test.result != expected)
        {
            std::cerr << "Incorrect result in "
                << toString(test.type)
                << ": got " << 
                sumToString(test.result)
                << ", expected " << 
                sumToString(expected)
                << "\n";
            return 1;
        }
    }


    // Вывод заголовка
    std::cout << std::setw(10) << "ID" 
              << std::setw(20) << "Type" 
              << std::setw(20) << "Time"
              << std::setw(30) << "Result" << "\n";

    // Линия-разделитель
    std::cout << std::string(100, '-') << "\n";

    printResult(tests);

    return 0;
}
