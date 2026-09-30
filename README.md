# Лабораторные: Windows и Linux

Команды выполняются из D:\Study\7_semestr\Parallel (на Linux — из папки проекта).
Поддерживаются GCC x64 под Linux и GCC из MSYS2 UCRT64 под Windows.
Для lab1_task1 нужен процессор с AVX2. MSVC не подходит из-за __int128.

## Windows

На этом компьютере зависимости уже установлены в C:\msys64.
На другом компьютере установите MSYS2 и выполните в терминале UCRT64:

```sh
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-opencv
```

В PowerShell выберите нужную команду:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File ./run.ps1 build
powershell -NoProfile -ExecutionPolicy Bypass -File ./run.ps1 lab1_task1
powershell -NoProfile -ExecutionPolicy Bypass -File ./run.ps1 lab1_task2
powershell -NoProfile -ExecutionPolicy Bypass -File ./run.ps1 lab2_seq
powershell -NoProfile -ExecutionPolicy Bypass -File ./run.ps1 lab2_omp
powershell -NoProfile -ExecutionPolicy Bypass -File ./run.ps1 task2_omp
```

lab1_task1 запросит n: введите, например, 1000000.
Собственное изображение: добавьте `-Image "C:\images\photo.jpg"` к команде lab2_seq/lab2_omp.
Другой путь MSYS2: добавьте `-MsysRoot "D:\msys64"`.
Скрипт сам временно настраивает PATH и останавливается при ошибке сборки.
Для обработки изображений в 14 потоках задайте перед запуском:
`$env:OMP_NUM_THREADS = '14'` и `$env:OMP_DYNAMIC = 'FALSE'`.

## Linux (Ubuntu / Debian)

```sh
sudo apt-get update
sudo apt-get install g++ cmake ninja-build libopencv-dev
bash run.sh build
bash run.sh lab1_task1
bash run.sh lab1_task2
bash run.sh lab2_seq
OMP_NUM_THREADS=14 OMP_DYNAMIC=FALSE bash run.sh lab2_omp
bash run.sh task2_omp
```

Собственное изображение: `bash run.sh lab2_omp /path/to/photo.jpg`.
Скрипты можно вызвать из другой папки, указав путь к скрипту.

## Сборка и результаты

Скрипты выполняют инкрементальную сборку Release перед запуском.
Исполняемые файлы: build-windows/bin/*.exe или build-linux/bin/*.
Для прямого запуска exe в PowerShell сначала выполните:
`$env:Path = "C:\msys64\ucrt64\bin;" + $env:Path`.
Старые бинарники и lab2/build из репозитория не используются:
кэши сборки нельзя переносить между Windows и Linux.

Результаты обработки изображения MR.yaml и result.png находятся в
build-windows/lab2/task1/lab2_seq-output и lab2_omp-output;
на Linux — аналогично внутри build-linux.
Повторный запуск перезаписывает результаты соответствующей версии.
Исходные результаты и папка Отчёты не изменяются.

Обычные матричные задачи используют 2048×2048 и 10 повторений.
Вычисления могут идти долго; таблица появляется в конце.

## Быстрая проверка корректности

```sh
cmake -S . -B build-smoke-linux -G Ninja -DCMAKE_BUILD_TYPE=Release -DLABS_SMOKE=ON
cmake --build build-smoke-linux --parallel
./build-smoke-linux/bin/lab1_task2
./build-smoke-linux/bin/task2_omp
```

Для Windows используйте build-smoke-windows и расширение .exe;
перед сборкой добавьте UCRT64 в начало PATH, как показано выше.
LABS_SMOKE включает матрицы 64×64 и один повтор, только для проверки.
Comparison должен быть 1. Несовпадение возвращает ненулевой код завершения.
Для замеров используйте обычные run.ps1/run.sh.
Без OpenCV можно собрать первую лабораторную отдельно:
`cmake -S . -B build-lab1 -G Ninja -DBUILD_LAB2=OFF -DCMAKE_BUILD_TYPE=Release`.
Затем `cmake --build build-lab1 --parallel`.

## Русский текст и терминал MSYS2

Программы и run.ps1 автоматически включают UTF-8. Скрипт восстанавливает
исходные настройки терминала после завершения.
В MSYS2 UCRT64 используйте прямой слеш в пути к скрипту:

```sh
powershell.exe -NoProfile -ExecutionPolicy Bypass -File ./run.ps1 lab2_omp
```

Если кодировка самого окна mintty была изменена вручную, верните
Options → Text → Character set → UTF-8.
