#pragma once

#include <float.h>
#include <math.h>
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

#define M_E 2.71828182845904523536         // e
#define M_LOG2E 1.44269504088896340736     // log2(e)
#define M_LOG10E 0.434294481903251827651   // log10(e)
#define M_LN2 0.693147180559945309417      // ln(2)
#define M_LN10 2.30258509299404568402      // ln(10)
#define M_PI 3.14159265358979323846        // pi
#define M_PI_2 1.57079632679489661923      // pi/2
#define M_PI_4 0.785398163397448309616     // pi/4
#define M_1_PI 0.318309886183790671538     // 1/pi
#define M_2_PI 0.636619772367581343076     // 2/pi
#define M_2_SQRTPI 1.12837916709551257390  // 2/sqrt(pi)
#define M_SQRT2 1.41421356237309504880     // sqrt(2)
#define M_SQRT1_2 0.707106781186547524401  // 1/sqrt(2)

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
