#pragma once

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

inline void initializeConsoleUtf8()
{
#ifdef _WIN32
    // Native consoles use UTF-8; redirected output and MSYS2 pipes already
    // receive the UTF-8 bytes produced by the program.
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif
}
