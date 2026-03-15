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
#else
#include <unistd.h>
#endif

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define MEASURE_FUNC_PERFORMANCE(func)                                     \
  do {                                                                     \
    clock_t start = clock();                                               \
    func;                                                                  \
    clock_t end = clock();                                                 \
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;               \
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
int32_t log_delete(void);

double get_high_resolution_time(void);
void mana_search_for_assets_directory(char* asset_directory, size_t max_len);
