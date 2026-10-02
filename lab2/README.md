# Лабораторная 2

Сборка и запуск полностью находятся в этой папке.
Нужны GCC, CMake, Ninja, OpenCV и OpenMP.
На этом компьютере Windows-зависимости уже установлены в C:/msys64.
На другом компьютере в MSYS2 UCRT64:
```sh
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-opencv
```
Из папки Parallel в MSYS2 UCRT64 или PowerShell:
```sh
powershell.exe -NoProfile -ExecutionPolicy Bypass -File ./lab2/run.ps1 build
powershell.exe -NoProfile -ExecutionPolicy Bypass -File ./lab2/run.ps1 lab2_seq
powershell.exe -NoProfile -ExecutionPolicy Bypass -File ./lab2/run.ps1 lab2_omp
powershell.exe -NoProfile -ExecutionPolicy Bypass -File ./lab2/run.ps1 task2_omp
```
Если терминал уже в lab2, используйте ./run.ps1.
Для своего изображения добавьте -Image C:/images/photo.jpg.
Скрипт автоматически настраивает UTF-8 и PATH.

Linux (Ubuntu/Debian):
```sh
sudo apt-get install g++ cmake ninja-build libopencv-dev
bash lab2/run.sh build
bash lab2/run.sh lab2_seq
OMP_NUM_THREADS=14 OMP_DYNAMIC=FALSE bash lab2/run.sh lab2_omp
bash lab2/run.sh task2_omp
```
Если терминал уже в lab2, используйте bash run.sh.
Своё изображение: bash lab2/run.sh lab2_omp /path/to/photo.jpg.

Сборки: lab2/build-windows/bin и lab2/build-linux/bin.
Результаты MR.yaml и result.png: build-windows/task1/lab2_seq-output и
build-windows/task1/lab2_omp-output; для Linux аналогично внутри build-linux.
Предыдущие результаты сохранены в build-previous-results.
Повторный запуск перезаписывает результаты соответствующей версии.
Матричная задача: 2048×2048, 10 повторений, таблица появляется в конце.
Быстрая проверка из lab2: cmake -S . -B build-smoke -G Ninja -DLABS_SMOKE=ON
Затем cmake --build build-smoke --parallel. Матрица 64×64, один повтор.
Для каждой ОС используйте отдельный каталог сборки.
Время вычислений во всех заданиях выводится в секундах (s).
