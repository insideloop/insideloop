#ifndef BENCHMARK_INTERNAL_MACROS_H_
#define BENCHMARK_INTERNAL_MACROS_H_

#include "benchmark/benchmark.h"

#ifndef __has_feature
#define __has_feature(x) 0
#endif

#if defined(__clang__)
#define COMPILER_CLANG
#elif defined(_MSC_VER)
#define COMPILER_MSVC
#elif defined(__GNUC__)
#define COMPILER_GCC
#endif

#if __has_feature(cxx_attributes)
#define BENCHMARK_NORETURN [[noreturn]]
#elif defined(__GNUC__)
#define BENCHMARK_NORETURN __attribute__((noreturn))
#elif defined(COMPILER_MSVC)
#define BENCHMARK_NORETURN __declspec(noreturn)
#else
#define BENCHMARK_NORETURN
#endif

#if defined(__CYGWIN__)
#define BENCHMARK_OS_CYGWIN 1
#elif defined(_WIN32)
#define BENCHMARK_OS_WINDOWS 1
#elif defined(__APPLE__)
#include "TargetConditionals.h"
#if defined(TARGET_OS_MAC)
#define BENCHMARK_OS_MACOSX 1
#if defined(TARGET_OS_IPHONE)
#define BENCHMARK_OS_IOS 1
#endif
#endif
#elif defined(__FreeBSD__)
#define BENCHMARK_OS_FREEBSD 1
#elif defined(__linux__)
#define BENCHMARK_OS_LINUX 1
#elif defined(__native_client__)
#define BENCHMARK_OS_NACL 1
#elif defined(EMSCRIPTEN)
#define BENCHMARK_OS_EMSCRIPTEN 1
#elif defined(__rtems__)
#define BENCHMARK_OS_RTEMS 1
#endif

#if !__has_feature(cxx_exceptions) && !defined(__cpp_exceptions) && \
    !defined(__EXCEPTIONS)
#define BENCHMARK_HAS_NO_EXCEPTIONS
#endif

#endif  // BENCHMARK_INTERNAL_MACROS_H_
