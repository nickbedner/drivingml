#include "mana/core/corecommon.h"

void log_message(enum log_severity_t severity, const char *format, ...) {
  va_list args;
  va_start(args, format);

  time_t t = time(NULL);
  struct tm tm;
  localtime_s(&tm, &t);
  char timestamp[26];
  asctime_s(timestamp, sizeof(timestamp), &tm);
  char *newline;
  strtok_s(timestamp, "\n", &newline);
  if (newline)
    *newline = '\0';

  const char *severity_str;
  const char *color_code;
  switch (severity) {
    case LOG_SEVERITY_DEBUG: {
#ifndef NDEBUG
      severity_str = "DEBUG";
      color_code = "";
#else
      return;
#endif
      break;
    }
    case LOG_SEVERITY_INFO: {
      severity_str = "INFO";
      color_code = "";
      break;
    }
    case LOG_SEVERITY_WARNING: {
      severity_str = "WARNING";
      color_code = "\033[33m";
      break;
    }
    case LOG_SEVERITY_ERROR: {
      severity_str = "ERROR";
      color_code = "\033[31m";
      break;
    }
    case LOG_SEVERITY_CRITICAL: {
      severity_str = "CRITICAL";
      color_code = "\033[36m";
      break;
    }
  }

  fprintf(stdout, "[%s] [%s%s\033[0m] ", timestamp, color_code, severity_str);
  // fputs(format, stdout);
  vfprintf(stdout, format, args);
  fflush(stdout);

  if (severity == LOG_SEVERITY_WARNING || severity == LOG_SEVERITY_ERROR || severity == LOG_SEVERITY_CRITICAL) {
    FILE *error_log;
    int result = fopen_s(&error_log, "error.log", "a");
    if (result != 0) {
      char error_message[256];
      strerror_s(error_message, sizeof(error_message), errno);
      fprintf(stderr, "Error opening error.log: %s\n", error_message);
      return;
    }

    fprintf(error_log, "[%s] [%s] ", timestamp, severity_str);
    // fputs(format, error_log);
    vfprintf(error_log, format, args);
    fflush(error_log);
    fclose(error_log);
  }
}

int32_t log_delete(void) {
  return _unlink("error.log");
}

double get_high_resolution_time(void) {
#ifdef _WIN64
  LARGE_INTEGER frequency;
  LARGE_INTEGER counter;
  QueryPerformanceFrequency(&frequency);
  QueryPerformanceCounter(&counter);
  return (double)counter.QuadPart / (double)frequency.QuadPart;
#else
  struct timespec current_time;
  clock_gettime(CLOCK_MONOTONIC, &current_time);
  long delta_seconds = current_time.tv_sec - prev_time.tv_sec;
  long delta_nanoseconds = current_time.tv_nsec - prev_time.tv_nsec;
  return (double)delta_seconds + (double)delta_nanoseconds / 1000000000;
#endif
}

// Function to check if a directory exists
static bool directory_exists(const wchar_t *path) {
  DWORD dwAttrib = GetFileAttributesW(path);
  return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

void mana_search_for_assets_directory(wchar_t *asset_directory) {
#ifdef _WIN64
  const wchar_t *folder_name = L"assets";
  wchar_t current_path[MAX_PATH];
  wchar_t parent_path[MAX_PATH];
  wchar_t final_path[MAX_PATH];

  // Check in current directory
  _snwprintf_s(current_path, MAX_PATH, MAX_PATH, L".\\%s", folder_name);
  if (directory_exists(current_path)) {
    wcscpy_s(final_path, MAX_PATH, current_path);
  } else {
    // Check in parent directory
    _snwprintf_s(parent_path, MAX_PATH, MAX_PATH, L"..\\%s", folder_name);
    if (directory_exists(parent_path)) {
      wcscpy_s(final_path, MAX_PATH, parent_path);
    } else {
      log_message(LOG_SEVERITY_CRITICAL, "The 'assets' folder was not found!\n");
      return;
    }
  }

  // Assuming asset_directory is large enough to hold the path
  wcscpy_s(asset_directory, MAX_PATH, final_path);
#endif
}
