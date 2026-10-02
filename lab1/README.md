# Лабораторная 1

Сборка и запуск полностью находятся в этой папке.
Windows: GCC x64 из MSYS2 UCRT64, CMake и Ninja. Для task1 нужен AVX2.
MSVC не поддерживается из-за __int128.

Из папки Parallel в MSYS2 UCRT64 или PowerShell:
```sh
powershell.exe -NoProfile -ExecutionPolicy Bypass -File ./lab1/run.ps1 build
powershell.exe -NoProfile -ExecutionPolicy Bypass -File ./lab1/run.ps1 lab1_task1
powershell.exe -NoProfile -ExecutionPolicy Bypass -File ./lab1/run.ps1 lab1_task2
```
Первая задача запросит n, например 1000000.
Если терминал уже в lab1, используйте ./run.ps1.
При другой установке MSYS2 добавьте -MsysRoot D:/msys64.
Скрипт автоматически настраивает UTF-8 и PATH.

Linux (Ubuntu/Debian):
```sh
sudo apt-get install g++ cmake ninja-build
bash lab1/run.sh build
bash lab1/run.sh lab1_task1
bash lab1/run.sh lab1_task2
```
Если терминал уже в lab1, используйте bash run.sh.

Сборки: lab1/build-windows/bin и lab1/build-linux/bin.
Обычная матричная задача: 2048×2048, 10 повторений, таблица в конце.
Быстрая проверка из lab1: cmake -S . -B build-smoke -G Ninja -DLABS_SMOKE=ON
Затем cmake --build build-smoke --parallel. Матрица 64×64, один повтор.
Для каждой ОС используйте отдельный каталог сборки.

Структура заданий:
- task1/main.cpp, task1/CMakeLists.txt — сумма ряда, SIMD и потоки.
- task2/main.cpp, task2/CMakeLists.txt — умножение матриц.
- task1/build-legacy и task2/build-legacy — сохранённые старые бинарники.
Общие скрипты, настройки и include находятся в папке lab1.
Актуальные программы собираются в build-windows/bin или build-linux/bin.
