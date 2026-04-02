#pragma once

#include "mana/core/corecommon.h"

// NOTE: Will only read binary file
// TODO: Pack this in a struct or something
global inline char* read_file(const char* filename) {
  char* data = NULL;
  FILE* fp;
  const int result = fopen_s(&fp, filename, "rb");
  if (result != 0) {
    log_message(LOG_SEVERITY_ERROR, "Error opening file: %s\n", filename);
    return data;
  }

  fseek(fp, 0, SEEK_END);
  i64 size = ftell(fp);
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

global inline u32 file_size(const char* filename) {
  FILE* fp;
  long size;

#ifdef _WIN32
  if (fopen_s(&fp, filename, "rb") != 0)
#else
  fp = fopen(filename, "rb");
  if (!fp)
#endif
  {
    log_message(LOG_SEVERITY_ERROR, "Error opening file: %s\n", filename);
    return 0;
  }

  if (fseek(fp, 0, SEEK_END) != 0) {
    fclose(fp);
    return 0;
  }

  size = ftell(fp);
  fclose(fp);

  if (size < 0) {
    return 0;
  }

  return (u32)size;
}

global inline u32* read_shader_file(const char* filename, uint_fast64_t* file_length) {
  u32* data = NULL;
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
  data = (u32*)_aligned_malloc((size_t)size, sizeof(u32));
#else
  data = (u32*)aligned_alloc(sizeof(u32), (size_t)size);
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

global inline void close_shader_file(u32* data) {
#ifdef _WIN64
  _aligned_free(data);
#else
  free(data);
#endif
}
