#pragma once

#include <stdalign.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#if defined(__arm__) || defined(__aarch64__)
#include <arm_neon.h>
#else
#include <immintrin.h>
#include <smmintrin.h>
#endif

#ifdef _WIN64
#define CINTERFACE
#define COBJMACROS
#define UNICODE
#include <Windows.h>
#include <direct.h>
#include <initguid.h>
#include <intrin.h>
#include <wchar.h>
#include <windowsx.h>
#define MAX_LENGTH_OF_PATH MAX_PATH
#else
#include <unistd.h>
#define MAX_LENGTH_OF_PATH 4096
#endif

#define global static
#define internal static
#define persist static

typedef float r32;
typedef double r64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef uint8_t b8;
typedef uint32_t b32;
#define TRUE 1
#define FALSE 0

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define MEASURE_FUNC_PERFORMANCE(func)                                     \
  do {                                                                     \
    clock_t start = clock();                                               \
    func;                                                                  \
    clock_t end = clock();                                                 \
    r64 elapsed = (r64)(end - start) / CLOCKS_PER_SEC;                     \
    printf("Function '%s' took %f seconds to execute.\n", #func, elapsed); \
    fflush(stdout);                                                        \
  } while (0)

enum log_severity_t {
  LOG_SEVERITY_DEBUG = 0,
  LOG_SEVERITY_INFO,
  LOG_SEVERITY_WARNING,
  LOG_SEVERITY_ERROR,
  LOG_SEVERITY_CRITICAL
};

void log_message(enum log_severity_t severity, const char* format, ...);
i32 log_delete(void);

r64 get_high_resolution_time(void);
void mana_search_for_assets_directory(char* asset_directory, size_t max_len);
