#!/usr/bin/env bash
set -euo pipefail
root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
task="${1:-build}"
case "$task" in
    build|lab1_task1|lab1_task2|lab2_seq|lab2_omp|task2_omp) ;;
    *) echo "Unknown task: $task" >&2; exit 2 ;;
esac
cmake -S "$root" -B "$root/build-linux" -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build "$root/build-linux" --parallel
if [[ "$task" == lab2_seq || "$task" == lab2_omp ]]; then
    "$root/build-linux/bin/$task" "${2:-$root/lab2/task1/images/1024x768.jpg}"
elif [[ "$task" != build ]]; then
    "$root/build-linux/bin/$task"
fi
