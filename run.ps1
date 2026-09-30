param(
    [ValidateSet('build','lab1_task1','lab1_task2','lab2_seq','lab2_omp','task2_omp')]
    [string]$Task = 'build',
    [string]$Image = '',
    [string]$MsysRoot = 'C:\msys64'
)
$ErrorActionPreference = 'Stop'
$previousPath = $env:Path
$previousInputEncoding = [Console]::InputEncoding
$previousOutputEncoding = [Console]::OutputEncoding
$previousPipelineEncoding = $OutputEncoding
try {
    $utf8 = New-Object System.Text.UTF8Encoding($false)
    [Console]::InputEncoding = $utf8
    [Console]::OutputEncoding = $utf8
    $OutputEncoding = $utf8
    $env:Path = (Join-Path $MsysRoot 'ucrt64\bin') + ';' + $env:Path
    $build = Join-Path $PSScriptRoot 'build-windows'
    & cmake -S $PSScriptRoot -B $build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=g++
    if ($LASTEXITCODE -ne 0) { throw 'CMake configuration failed. See README.md for dependencies.' }
    & cmake --build $build --parallel
    if ($LASTEXITCODE -ne 0) { throw 'Build failed.' }
    if ($Task -ne 'build') {
        $exe = Join-Path $build "bin\$Task.exe"
        if ($Task -in @('lab2_seq','lab2_omp')) {
            if (-not $Image) { $Image = Join-Path $PSScriptRoot 'lab2\task1\images\1024x768.jpg' }
            & $exe $Image
        } else { & $exe }
        if ($LASTEXITCODE -ne 0) { throw "Program failed: $LASTEXITCODE" }
    }
} finally {
    $env:Path = $previousPath
    [Console]::InputEncoding = $previousInputEncoding
    [Console]::OutputEncoding = $previousOutputEncoding
    $OutputEncoding = $previousPipelineEncoding
}
