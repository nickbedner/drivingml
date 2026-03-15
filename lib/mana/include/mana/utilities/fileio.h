#pragma once

#include "mana/core/corecommon.h"

// NOTE: Will only read binary file
// TODO: Pack this in a struct or something
static inline char* read_file(const char* filename) {
  char* data = NULL;
  FILE* fp;
  const int result = fopen_s(&fp, filename, "rb");
  if (result != 0) {
    log_message(LOG_SEVERITY_ERROR, "Error opening file: %s\n", filename);
    return data;
  }

  fseek(fp, 0, SEEK_END);
  int64_t size = ftell(fp);
  rewind(fp);

  // NOTE: Could not reach end of file
  if (size == -1) {
    fclose(fp);
    return data;
  }

  data = (char*)malloc((size_t)size + 1);
  data[size] = '\0';

  // NOTE: Could not read chunk of binary data
  if (fread(data, (size_t)size, 1, fp) != 1) {
    free(data);
    data = NULL;
  }

  fclose(fp);
  return data;
}

static inline uint32_t* read_shader_file(const char* filename, uint_fast64_t* file_length) {
  uint32_t* data = NULL;
  FILE* fp;

#ifdef _WIN32
  if (fopen_s(&fp, filename, "rb") != 0)
#else
  fp = fopen(filename, "rb");
  if (!fp)
#endif
  {
    log_message(LOG_SEVERITY_ERROR, "Error opening file: %s\n", filename);
    return NULL;
  }

  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  rewind(fp);

  if (size <= 0) {
    fclose(fp);
    return NULL;
  }

  *file_length = (uint_fast64_t)size;

#ifdef _WIN32
  data = (uint32_t*)_aligned_malloc((size_t)size, sizeof(uint32_t));
#else
  data = (uint32_t*)aligned_alloc(sizeof(uint32_t), (size_t)size);
#endif

  if (!data) {
    fclose(fp);
    return NULL;
  }

  if (fread(data, 1, (size_t)size, fp) != (size_t)size) {
#ifdef _WIN32
    _aligned_free(data);
#else
    free(data);
#endif
    fclose(fp);
    return NULL;
  }

  fclose(fp);
  return data;
}

static inline void close_shader_file(uint32_t* data) {
#ifdef _WIN64
  _aligned_free(data);
#else
  free(data);
#endif
}
