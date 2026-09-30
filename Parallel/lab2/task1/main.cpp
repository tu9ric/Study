#include <opencv2/opencv.hpp>
#include <cmath>
#include <iostream>
#include <chrono>
#include <filesystem>

/*
В Vec3b объекте хранится массив в формате BGR
cv::Vec3b& pixel = image.at<cv::Vec3b>(y, x);
pixel[0] - blue
pixel[1] - green
pixel[2] - red
*/      

constexpr std::size_t N = 10;

/// @brief функция для нахождения матрицы интенсивности
/// @param image исходное изображение, для которого вычисляется матрица интенсивности
/// @return матрица интенсивности исходного изображения
cv::Mat makeIntensityMatrix(const cv::Mat& image)
{
    cv::Mat intensity(image.rows, image.cols, CV_8UC1);

    // распределяет итерации след цикла между потоками
    #ifdef _OPENMP
    #pragma omp parallel for schedule(static)
    #endif
    for (int y = 0; y < image.rows; ++y)
    {
        for (int x = 0; x < image.cols; ++x)
        {
            const cv::Vec3b& pixel = image.at<cv::Vec3b>(y, x);

            double value = (pixel[0] + pixel[1] + pixel[2]) / 3.0;

            intensity.at<unsigned char>(y, x) = cv::saturate_cast<unsigned char>(value);
        }
    }
    return intensity;
}

/// @brief функция для фильтрации матрицы интенсивности фильтром собеля
/// @param intensity матрица интенсивности
/// @param MX матрица отражающая изменение интенсивности по горизонтали, слева направо
/// @param MY матрица отражающая изменение интенсивности по вертикали, сверху вниз  
void sobelFilter(const cv::Mat& intensity, cv::Mat& MX, cv::Mat& MY)
{
    // проверка входных данных (на то что матрица не пустая и что в каждом пикселе один канал)
    CV_Assert(!intensity.empty() && intensity.type() == CV_8UC1);

    // инициализация матриц нулями
    // CV_32FC1 - каждый элемент 32-х битный float, один канал 
    MX = cv::Mat::zeros(intensity.size(), CV_32FC1);
    MY = cv::Mat::zeros(intensity.size(), CV_32FC1);

    // ядро для горизонтальной производной
    const int kernelX[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };

    // ядро для вертикальной производной
    const int kernelY[3][3] = {
        {-1, -2, -1},
        { 0,  0,  0},
        { 1,  2,  1}
    };


    #ifdef _OPENMP
    #pragma omp parallel for schedule(static)
    #endif
    // первую и последнюю строки и столбцы пропускаем, они остаются нулевыми
    // обход строк матрицы интенсивности
    for (int y = 1; y < intensity.rows - 1; ++y)
    {
        // обход столбцов матрицы интенсивности
        for (int x = 1; x < intensity.cols - 1; ++x)
        {
            // результаты применения ядер к окрестности пикселя
            int gx = 0;
            int gy = 0;

            // обход соседних строк
            // dy - смещение относительно текущей строки
            // -1 - выше текущего пикселя, 0 - текущая строка, 1 - строка ниже текущего пикселя
            for (int dy = -1; dy <= 1; ++dy)
            {
                // обход соседних столбцов
                // dx - смещение относительно текущего столбца
                // -1 - слева, 0 - текущий, 1 - справа
                for (int dx = -1; dx <= 1; ++dx)
                {
                    // чтение интенсивности соседнего пикселя
                    int value = intensity.at<unsigned char>(y + dy, x + dx);

                    // накопление горизонтальной производной
                    gx += value * kernelX[dy + 1][dx + 1];
                    // накопление вертикальной производной
                    gy += value * kernelY[dy + 1][dx + 1];
                }
            }
            // запись результатов
            MX.at<float>(y, x) = static_cast<float>(gx);
            MY.at<float>(y, x) = static_cast<float>(gy); 
        }
    }
}

/// @brief функция, которая формирует матрицу величин градиента Собеля
/// @param MX матрица отражающая изменение интенсивности по горизонтали, слева направо
/// @param MY матрица отражающая изменение интенсивности по вертикали, сверху вниз
/// @return матрица величин градиента Собеля
cv::Mat makeMR(const cv::Mat& MX, cv::Mat& MY)
{
    CV_Assert(!MX.empty() && MX.size() == MY.size());
    CV_Assert(MX.type() == CV_32FC1 && MY.type() == CV_32FC1);

    cv::Mat MR(MX.size(), CV_32FC1);

    #ifdef _OPENMP
    #pragma omp parallel for schedule(static)
    #endif
    for (int y = 0; y < MX.rows; ++y)
    {
        for (int x = 0; x < MX.cols; ++x)
        {
            float gx = MX.at<float>(y, x);
            float gy = MY.at<float>(y, x);

            MR.at<float>(y, x) = std::sqrt(gx * gx + gy * gy);
        }
    }
    return MR;
}

/// @brief функция нахождения максимального элемента в матрице
/// @param MR входная матрица
/// @return 
float maxMatrixValue(const cv::Mat& matrix)
{
    CV_Assert(!matrix.empty() && matrix.type() == CV_32FC1);
    float maxValue = matrix.at<float>(0, 0);

    // у каждого потока свой собственный накопитель, после цикла openMP объединит результаты
    #ifdef _OPENMP
    #pragma omp parallel for schedule(static) reduction(max:maxValue)
    #endif
    for (int y = 0; y < matrix.rows; ++y)
    {
        for (int x = 0; x < matrix.cols; ++x)
        {
            float value = matrix.at<float>(y, x);
            
            maxValue = (value > maxValue ? value : maxValue);
        }
    }
    return maxValue;
}

/// @brief функция для модификации матрицы в соответствии с требованием задания
/// @param matrix 
/// @param maxValue 
void modifyValue(cv::Mat& matrix, float maxValue)
{
    if (maxValue == 0.0f)
    {
        return;
    }

    const float scale = 255.0f / maxValue;

    #ifdef _OPENMP
    #pragma omp parallel for schedule(static)
    #endif
    for (int y = 0; y < matrix.rows; ++y)
    {
        for (int x = 0; x < matrix.cols; ++x)
        {  
            matrix.at<float>(y, x) *= scale;
        }
    }
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Использование: " << argv[0]
        << " <путь к изображению>\n";
        return 1;
    }

    // CMake передаёт путь к lab2/task1 независимо от папки запуска.
    const std::filesystem::path outputDirectory(TASK1_OUTPUT_DIR);
    const auto matrixPath = outputDirectory / "MR.yaml";
    const auto imagePath = outputDirectory / "result.png";

    for(std::size_t i = 0; i < N; ++i)
    {
        // изображение хранится в формате cv::Mat
        cv::Mat image = cv::imread(argv[1]);

        // проверка на то, что изображение не пустое
        if(image.empty())
        {
            std::cerr << "Ну удалось загрузить изображение\n";
            return 1;
        }

        const auto start = std::chrono::steady_clock::now();

        // вычисление матрицы интенсивности для исходного изображения
        cv::Mat imageIntensity = makeIntensityMatrix(image);

        // фильтруем матрицу интенсивности
        cv::Mat MX, MY;
        sobelFilter(imageIntensity, MX, MY);

        // формирование матрицы величин градиента
        cv::Mat MR = makeMR(MX, MY);

        // нахождение максимального значения в матрице
        float maxMRValue = maxMatrixValue(MR);
    
        // модификация матрицы в соответствии с заданием
        modifyValue(MR, maxMRValue);

        const auto finish = std::chrono::steady_clock::now();
        const double timeMs = std::chrono::duration<double, std::milli>(finish - start).count();

        if(i == 0)
        {
            std::cout << "Максимальный элемент MR: " << maxMRValue << '\n';
        }
        
        std::cout << "Время вычислений: " << timeMs << " Ms\n";

        // сохранение модифицированной матрицы MR в yaml файл, так как пиксели сейчас хранятся во float
        // MR имеет тип CV_32FC1 и может содержать дробные числа
        cv::FileStorage file(matrixPath.string(), cv::FileStorage::WRITE);
        if (!file.isOpened())
        {
            std::cerr << "Не удалось открыть файл для записи\n";
            return 1;
        }
        file << "MR" << MR;
        file.release();
        
        // конвертация матрицы MR и её вывод в png
        cv::Mat result;
        MR.convertTo(result, CV_8UC1);
        if (!cv::imwrite(imagePath.string(), result))
        {
            std::cerr << "Не удалось сохранить изображение\n";
            return 1;
        }
    }


    //cv::imshow("Image", result);
    //cv::waitKey(0);
}
