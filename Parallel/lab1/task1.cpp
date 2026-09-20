#include <iostream>
#include <thread>
#include <chrono>
#include <array>
#include <iomanip>
#include <vector>
#include <atomic>
// библиотека для работы с SIMD в C++
#include <immintrin.h>

constexpr int NUM_OF_THREADS = 14;
constexpr std::size_t NUM_OF_TESTS = 10;

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
    long long result;
};

/// @brief функция для сложения чисел 
/// @param begin первое значение диапазона
/// @param end первая граница диапазона
/// @return сумма чисел, находящихся в отрезке между begin и end
long long sumOfRange(long long begin, long long end)
{
    long long sum {0};
    for(long long i = begin; i <= end; ++i)
    {
        sum += i;
    }
    return sum;
}

// будем использовать 256-битные целочисленные AVX2 регистры __m256i
/// @brief сумма ряда через simd инструкции
/// @param begin 
/// @param end 
/// @return 
long long sumOfRangeSIMD(const long long begin, const long long end)
{
    // это сумма значений
    __m256i sumSIMD = _mm256_setzero_si256();

    __m256i valuesSIMD = _mm256_setr_epi64x(begin, begin + 1, begin + 2, begin + 3);

    // шаг, потому что мы складываем 4 числа, а не по одному
    __m256i stepSIMD = _mm256_set1_epi64x(4);

    long long i = begin;
    for (; i + 3 <= end; i += 4)
    {
        sumSIMD = _mm256_add_epi64(sumSIMD, valuesSIMD);
        valuesSIMD = _mm256_add_epi64(valuesSIMD, stepSIMD);
    }

    // считаем суммы хвоста (если он есть)
    long long tailSum{0};
    for(; i <= end; ++i)
    {
        tailSum += i;
    }

    long long partialSums[4];

    // использую storeu, чтобы не заниматься выравниванием памяти
    // нам нужно привести partialSums к указателю на __m256i, как этого требует функция AVX2
    _mm256_storeu_si256(reinterpret_cast<__m256i*>(partialSums), sumSIMD);

    // после этого шага, у меня в partialSums будет 4 отдельных частичных суммы
    // останется только их сложить

    // сразу складываю в переменную хвост, чтобы потом не прибавлять
    long long sum{tailSum};
    for (long long elem : partialSums)
    {
        sum += elem;
    }
    return sum;
} 



Test LinearTest(long long n)
{
    Test LinearTest{};
    auto start = std::chrono::steady_clock::now();
    long long res = sumOfRange(0, n);
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> LinearTime = end - start;
    LinearTest.id = IDGenerator::generate();
    LinearTest.type = TypeOfTest::Linear;
    LinearTest.time = LinearTime.count();
    LinearTest.result = res;
    return LinearTest;
}

Test SimdTest(long long n)
{
    Test SIMDTest{};
    auto start = std::chrono::steady_clock::now();
    long long res = sumOfRangeSIMD(0, n);
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
    std::array<long long, NUM_OF_THREADS> partial{};
    // массив со всеми потоками
    std::array<std::thread, NUM_OF_THREADS> threads;

    // цикл в котором мы присваиваем каждому потоку свою область действия
    // тут мы определяем границы каждой из областей
    for (std::size_t t = 0; t < NUM_OF_THREADS; ++t)
    {
        // просчитываем начальное значение области
        const long long begin = (n + 1) * t / NUM_OF_THREADS;
        // и конечное значение области
        const long long end = (n + 1) * (t + 1) / NUM_OF_THREADS - 1;
        
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

    long long parallelRes = 0;
    for (long long value : partial)
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
    std::array<long long, NUM_OF_THREADS> partial{};
    // массив со всеми потоками
    std::array<std::thread, NUM_OF_THREADS> threads;

    // цикл в котором мы присваиваем каждому потоку свою область действия
    // тут мы определяем границы каждой из областей
    for (int t = 0; t < NUM_OF_THREADS; ++t)
    {
        // просчитываем начальное значение области
        const long long begin = (n + 1) * t / NUM_OF_THREADS;
        // и конечное значение области
        const long long end = (n + 1) * (t + 1) / NUM_OF_THREADS - 1;
        
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

    long long parallelRes = 0;
    for (long long value : partial)
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

void printResult(const std::vector<Test>& tests)
{
    for(auto& test : tests)
    {
        // Вывод строк таблицы
        std::cout << std::setw(10) << test.id
                  << std::setw(15) << toString(test.type)
                  << std::setw(10) << test.time 
                  << std::setw(20) << test.result << "\n";
    }     
}

int main()
{
    
    // добавил std::cin, чтобы были корректные замеры с -O2 оптимизацией
    // иначе компилятор заранее знает границы и выдаёт ответ сразу, что ломает эксперимент
    long long n;
    std::cout << "Enter n: ";
    std::cin >> n;
    //constexpr long long n = 1000000000;
    
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

    // Вывод заголовка
    std::cout << std::setw(10) << "ID" 
              << std::setw(15) << "Type" 
              << std::setw(10) << "Time"
              << std::setw(20) << "Result" << "\n";

    // Линия-разделитель
    std::cout << std::string(60, '-') << "\n";

    printResult(tests);

    return 0;
}
