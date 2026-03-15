#include "mana/core/corecommon.h"

void log_message(enum log_severity_t severity, const char* format, ...) {
  va_list args;
  va_start(args, format);

  time_t t = time(NULL);
  struct tm tm;
  localtime_s(&tm, &t);
  char timestamp[26];
  asctime_s(timestamp, sizeof(timestamp), &tm);
  char* newline;
  strtok_s(timestamp, "\n", &newline);
  if (newline)
    *newline = '\0';

  const char* severity_str;
  const char* color_code;
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
    FILE* error_log;
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

static bool directory_exists(const char* path) {
#ifdef _WIN32
  DWORD dwAttrib = GetFileAttributesA(path);
  return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#else
  struct stat info;
  if (stat(path, &info) != 0)
    return false;
  return (info.st_mode & S_IFDIR) != 0;
#endif
}

void mana_search_for_assets_directory(char* asset_directory, size_t max_len) {
  const char* folder_name = "assets";
  char current_path[MAX_PATH] = {0};
  char parent_path[MAX_PATH] = {0};

  char cwd[MAX_PATH] = {0};
#ifdef _WIN32
  _getcwd(cwd, sizeof(cwd));
#else
  getcwd(cwd, sizeof(cwd));
#endif

  log_message(LOG_SEVERITY_DEBUG, "Current working directory: %s\n", cwd);

  snprintf(current_path, sizeof(current_path), "./%s", folder_name);

  if (directory_exists(current_path)) {
    snprintf(asset_directory, max_len, "%s", current_path);
    return;
  }

  snprintf(parent_path, sizeof(parent_path), "../%s", folder_name);

  if (directory_exists(parent_path)) {
    snprintf(asset_directory, max_len, "%s", parent_path);
    return;
  }

  log_message(LOG_SEVERITY_CRITICAL, "The 'assets' folder was not found!\n");
}
