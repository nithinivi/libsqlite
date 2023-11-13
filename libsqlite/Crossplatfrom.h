#pragma once
#if defined(_WIN32)
// Microsoft Visual C++ compiler
#define NORETURN_ATTRIBUTE __declspec(noreturn)
#elif defined(__GNUC__)
// GCC or Clang compiler
#define NORETURN_ATTRIBUTE __attribute__((noreturn))
#else
// Add support for other compilers/platforms if needed
#define NORETURN_ATTRIBUTE
#endif