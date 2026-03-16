#include "mana/graphics/utilities/texture/texture.h"

#define PNG_SIGNATURE_SIZE 8

// Helper function for Paeth filter
static unsigned char paeth_predictor(int a, int b, int c) {
  int p = a + b - c;
  int pa = abs(p - a);
  int pb = abs(p - b);
  int pc = abs(p - c);
  if (pa <= pb && pa <= pc)
    return a;
  else if (pb <= pc)
    return b;
  else
    return c;
}

// Function to reverse PNG filtering for a single scanline
static void unfilter_scanline(uint8_t* recon, const uint8_t* scanline, const uint8_t* prev, size_t bytewidth, size_t length, uint8_t filter_type) {
  size_t i;

  switch (filter_type) {
    case 0: {  // None
      for (i = 0; i < length; i++)
        recon[i] = scanline[i];
      break;
    }
    case 1: {  // Sub
      for (i = 0; i < bytewidth; i++)
        recon[i] = scanline[i];
      for (i = bytewidth; i < length; i++)
        recon[i] = scanline[i] + recon[i - bytewidth];
      break;
    }
    case 2: {  // Up
      if (prev) {
        for (i = 0; i < length; i++)
          recon[i] = scanline[i] + prev[i];
      } else {
        for (i = 0; i < length; i++)
          recon[i] = scanline[i];
      }
      break;
    }
    case 3: {  // Average
      if (prev) {
        for (i = 0; i < bytewidth; i++)
          recon[i] = scanline[i] + (prev[i] / 2);
        for (i = bytewidth; i < length; i++)
          recon[i] = scanline[i] + ((recon[i - bytewidth] + prev[i]) / 2);
      } else {
        for (i = 0; i < bytewidth; i++)
          recon[i] = scanline[i];
        for (i = bytewidth; i < length; i++)
          recon[i] = scanline[i] + (recon[i - bytewidth] / 2);
      }
      break;
    }
    case 4: {  // Paeth
      if (prev) {
        for (i = 0; i < bytewidth; i++)
          recon[i] = scanline[i] + paeth_predictor(0, prev[i], 0);
        for (i = bytewidth; i < length; i++)
          recon[i] = scanline[i] + paeth_predictor(recon[i - bytewidth], prev[i], prev[i - bytewidth]);
      } else {
        for (i = 0; i < bytewidth; i++)
          recon[i] = scanline[i];
        for (i = bytewidth; i < length; i++)
          recon[i] = scanline[i] + paeth_predictor(recon[i - bytewidth], 0, 0);
      }
      break;
    }
    default: {
      fprintf(stderr, "Unknown PNG filter type encountered.\n");
      // exit(EXIT_FAILURE);
    }
  }
}

// Main function to reverse PNG filtering for the whole image
static uint8_t* reverse_png_filtering(uint8_t* data, uint32_t width, uint32_t height, uint32_t bpp, uint8_t color_type) {
  size_t linebytes = width * bpp;

  uint8_t* prev_line = NULL;
  uint8_t* pixels = calloc(1, linebytes * height);
  uint8_t* start_pos = pixels;
  if (!pixels) {
    fprintf(stderr, "Memory allocation failed\n");
    exit(EXIT_FAILURE);
  }

  for (uint32_t y = 0; y < height; y++) {
    uint8_t filter_type = *data++;  // First byte of a scanline is the filter type

    unfilter_scanline(pixels, data, prev_line, bpp, linebytes, filter_type);

    // Set the previous line to the current one for the next iteration
    prev_line = pixels;

    // Move to the next line
    data += linebytes;
    pixels += linebytes;
  }

  pixels = start_pos;

  // if (color_type == 0) {
  //   uint8_t *new_pixels = calloc(1, width * height * 4);
  //   uint8_t *new_pixels_start_pos = new_pixels;
  //   for (uint32_t y = 0; y < height; y++) {
  //     for (uint32_t x = 0; x < width; x++) {
  //       new_pixels[0] = *pixels;
  //       new_pixels[1] = *pixels;
  //       new_pixels[2] = *pixels;
  //       new_pixels[3] = UCHAR_MAX;
  //       new_pixels += 4;
  //       pixels++;
  //     }
  //   }
  //   pixels = start_pos;
  //   free(pixels);
  //   pixels = new_pixels_start_pos;
  // } else if (color_type == 2) {
  if (color_type == 2) {
    uint8_t* new_pixels = calloc(1, width * height * 4);
    uint8_t* new_pixels_start_pos = new_pixels;
    for (uint32_t y = 0; y < height; y++) {
      for (uint32_t x = 0; x < width; x++) {
        new_pixels[0] = pixels[0];
        new_pixels[1] = pixels[1];
        new_pixels[2] = pixels[2];
        new_pixels[3] = UCHAR_MAX;
        new_pixels += 4;
        pixels += 3;
      }
    }
    pixels = start_pos;
    free(pixels);
    pixels = new_pixels_start_pos;
  }

  return pixels;
}

static inline uint16_t ntohs(uint16_t netshort) {
  return netshort;
  // return ((netshort >> 8) & 0xff) | ((netshort & 0xff) << 8);
}
static inline uint16_t ntohs2(uint16_t netshort) {
  uint8_t cur[2];
  cur[0] = netshort & 0xff;
  cur[1] = netshort >> 8;
  return (cur[0] << 8) | cur[1];
}

static int paeth_predictor_16(int a, int b, int c) {
  int p = a + b - c;
  int pa = abs(p - a);
  int pb = abs(p - b);
  int pc = abs(p - c);
  if (pa <= pb && pa <= pc)
    return a;
  if (pb <= pc)
    return b;
  return c;
}

// Check top byte whether to wrap or not
static uint16_t calculate_wrap_around(uint16_t val1, uint16_t val2, uint8_t byte_val1, uint8_t byte_val2) {
  // uint8_t clamp_val = (byte_val1 + byte_val2) % UCHAR_MAX;
  uint8_t clamp_val = byte_val1 + byte_val2;
  uint16_t corrected_short = clamp_val * 256;
  int added_val = val1 + val2;
  if (added_val > corrected_short + UCHAR_MAX)
    return corrected_short + UCHAR_MAX;
  if (added_val < corrected_short)
    return corrected_short;

  return added_val;
}

// Function to reverse PNG filtering for a single scanline
static void unfilter_scanline_16bit(uint16_t* recon, const uint16_t* scanline, const uint16_t* prev, const uint8_t* prev_byte_line, uint8_t* byte_pixels, uint8_t* byte_data, size_t shortwidth, size_t length, uint8_t filter_type) {
  size_t i;

  switch (filter_type) {
      // Note: This doesn't get called
    case 0: {  // None
      for (i = 0; i < length; i++) {
        recon[i] = scanline[i];
        byte_pixels[i] = byte_data[i];
      }
      break;
    }
      // Note: This gets called
    case 1: {  // Sub
      for (i = 0; i < shortwidth; i++) {
        recon[i] = scanline[i];
        byte_pixels[i] = byte_data[i];
      }
      for (i = shortwidth; i < length; i++) {
        recon[i] = calculate_wrap_around(scanline[i], recon[i - shortwidth], byte_data[i], byte_pixels[i - shortwidth]);
        byte_pixels[i] = byte_data[i] + byte_pixels[i - shortwidth];
      }
      break;
    }
      // Note: Only first case gets called
    case 2: {  // Up
      if (prev) {
        for (i = 0; i < length; i++) {
          recon[i] = calculate_wrap_around(scanline[i], prev[i], byte_data[i], prev_byte_line[i]);
          byte_pixels[i] = byte_data[i] + prev_byte_line[i];
        }
      } else {
        for (i = 0; i < length; i++) {
          recon[i] = scanline[i];
          byte_pixels[i] = byte_data[i];
        }
      }
      break;
    }
      // Note: This doesn't get called
    case 3: {  // Average
      if (prev) {
        for (i = 0; i < shortwidth; i++)
          recon[i] = scanline[i] + (prev[i] / 2);
        for (i = shortwidth; i < length; i++)
          recon[i] = scanline[i] + ((recon[i - shortwidth] + prev[i]) / 2);
      } else {
        for (i = 0; i < shortwidth; i++)
          recon[i] = scanline[i];
        for (i = shortwidth; i < length; i++)
          recon[i] = scanline[i] + (recon[i - shortwidth] / 2);
      }
      break;
    }
    case 4: {  // Paeth
      if (prev) {
        for (i = 0; i < shortwidth; i++) {
          recon[i] = calculate_wrap_around(scanline[i], paeth_predictor_16(0, prev[i], 0), byte_data[i], paeth_predictor(0, prev_byte_line[i], 0));
          byte_pixels[i] = byte_data[i] + paeth_predictor(0, prev_byte_line[i], 0);
        }
        for (i = shortwidth; i < length; i++) {
          recon[i] = calculate_wrap_around(scanline[i], paeth_predictor_16(recon[i - shortwidth], prev[i], prev[i - shortwidth]), byte_data[i], paeth_predictor(byte_pixels[i - shortwidth], prev_byte_line[i], prev_byte_line[i - shortwidth]));
          byte_pixels[i] = byte_data[i] + paeth_predictor(byte_pixels[i - shortwidth], prev_byte_line[i], prev_byte_line[i - shortwidth]);
        }
      } else {
        for (i = 0; i < shortwidth; i++) {
          recon[i] = scanline[i];
          byte_pixels[i] = byte_data[i];
        }
        for (i = shortwidth; i < length; i++) {
          recon[i] = calculate_wrap_around(scanline[i], paeth_predictor_16(recon[i - shortwidth], 0, 0), byte_data[i], paeth_predictor(byte_pixels[i - shortwidth], 0, 0));
          byte_pixels[i] = byte_data[i] + paeth_predictor(byte_pixels[i - shortwidth], 0, 0);
        }
      }
      break;
    }
    default: {
      fprintf(stderr, "Unknown PNG filter type encountered.\n");
      // exit(EXIT_FAILURE);
    }
  }
}

static uint16_t* reverse_png_filtering_16bit(uint8_t* data, uint32_t width, uint32_t height, uint32_t bpp, uint8_t color_type) {
  size_t linebytes = width * bpp;  // Bytes per line

  uint16_t* prev_line = NULL;
  uint8_t* prev_byte_line = NULL;
  // uint16_t *pixels = calloc(1, linebytes * height);
  // uint16_t *pixels = (uint16_t *)_aligned_malloc(linebytes * height, alignof(uint16_t));
  // uint16_t *aligned_data = (uint16_t *)_aligned_malloc(linebytes * height, alignof(uint16_t));
  // uint8_t *byte_pixels = (uint8_t *)_aligned_malloc((linebytes * height) / 2, alignof(uint8_t));
  // uint8_t *byte_data = (uint8_t *)_aligned_malloc((linebytes * height) / 2, alignof(uint8_t));
  uint16_t* pixels = (uint16_t*)malloc(linebytes * height);
  uint16_t* aligned_data = (uint16_t*)malloc(linebytes * height);
  uint8_t* byte_pixels = (uint8_t*)malloc((linebytes * height) / 2);
  uint8_t* byte_data = (uint8_t*)malloc((linebytes * height) / 2);
  uint16_t* start_pos = pixels;
  uint16_t* data_start_pos = aligned_data;
  uint8_t* byte_pixels_start_pos = byte_pixels;
  uint8_t* byte_data_start_pos = byte_data;

  // memset(pixels, 0, linebytes * height);
  // memset(aligned_data, 0, linebytes * height);
  // memset(byte_data, 0, (linebytes * height) / 2);
  // memset(byte_pixels, 0, (linebytes * height) / 2);

  uint8_t* data_start = data;
  for (uint32_t y = 0; y < height; y++) {
    uint8_t* pixel_data = data_start + 1;

    for (uint32_t i = 0; i < width * bpp / 2; i++) {
      uint8_t high_byte, low_byte;
      memcpy(&high_byte, pixel_data + i * 2, 1);
      memcpy(&low_byte, pixel_data + i * 2 + 1, 1);
      aligned_data[i] = (uint16_t)((high_byte << 8) | low_byte);
      byte_data[i] = high_byte;
    }

    data_start += linebytes + 1;
    aligned_data += width * (bpp / 2);
    byte_data += width * (bpp / 2);
  }
  aligned_data = data_start_pos;
  byte_data = byte_data_start_pos;

  if (!pixels) {
    fprintf(stderr, "Memory allocation failed\n");
    exit(EXIT_FAILURE);
  }

  for (uint32_t y = 0; y < height; y++) {
    uint8_t filter_type = *data++;  // Read filter type as 8-bit value

    unfilter_scanline_16bit(pixels, aligned_data, prev_line, prev_byte_line, byte_pixels, byte_data, bpp / 2, linebytes / 2, filter_type);

    // Set the previous line to the current one for the next iteration
    prev_line = pixels;
    prev_byte_line = byte_pixels;

    // Move to the next line
    data += linebytes;
    pixels += linebytes / 2;
    aligned_data += linebytes / 2;
    byte_data += linebytes / 2;
    byte_pixels += linebytes / 2;
  }

  pixels = start_pos;

  if (color_type == 0) {
    uint16_t* new_pixels = calloc(1, width * height * 8);
    uint16_t* new_pixels_start_pos = new_pixels;
    for (uint32_t y = 0; y < height; y++) {
      for (uint32_t x = 0; x < width; x++) {
        new_pixels[0] = *pixels;
        new_pixels[1] = *pixels;
        new_pixels[2] = *pixels;
        new_pixels[3] = USHRT_MAX;
        new_pixels += 4;
        pixels++;
      }
    }
    free(start_pos);
    pixels = new_pixels_start_pos;
  } else if (color_type == 2) {
    uint16_t* new_pixels = calloc(1, width * height * 8);
    uint16_t* new_pixels_start_pos = new_pixels;
    for (uint32_t y = 0; y < height; y++) {
      for (uint32_t x = 0; x < width; x++) {
        new_pixels[0] = pixels[0];
        new_pixels[1] = pixels[1];
        new_pixels[2] = pixels[2];
        new_pixels[3] = USHRT_MAX;
        new_pixels += 4;
        pixels += 3;
      }
    }
    free(start_pos);
    pixels = new_pixels_start_pos;
  }

  free(data_start_pos);
  free(byte_pixels_start_pos);
  free(byte_data_start_pos);

  return pixels;
}

// Manually define a simple byte-swapping function for uint32_t
static uint32_t swap_uint32(uint32_t val) {
  return ((val << 24) & 0xff000000) |
         ((val << 8) & 0x00ff0000) |
         ((val >> 8) & 0x0000ff00) |
         ((val >> 24) & 0x000000ff);
}

// Function to concatenate IDAT chunks
static unsigned char* concatenate_idat_chunks(FILE* fp, unsigned int* total_length) {
  unsigned int buffer_size = 1024;  // Initial buffer size
  unsigned char* buffer = malloc(buffer_size);
  if (buffer == NULL) {
    fprintf(stderr, "Memory allocation failed\n");
    exit(EXIT_FAILURE);
  }

  *total_length = 0;  // Initialize total length

  // Read each chunk
  while (!feof(fp)) {
    uint32_t length;
    unsigned char type[4];
    uint32_t crc;

    // Read the length of the chunk
    fread(&length, 4, 1, fp);
    length = swap_uint32(length);

    // Read the chunk type
    fread(type, 1, 4, fp);

    if (strncmp((const char*)type, "IDAT", 4) == 0) {
      // Resize buffer if needed
      if (*total_length + length > buffer_size) {
        buffer_size = *total_length + length;
        buffer = realloc(buffer, buffer_size);
        if (buffer == NULL) {
          fprintf(stderr, "Memory reallocation failed\n");
          exit(EXIT_FAILURE);
        }
      }

      // Read IDAT chunk data
      fread(buffer + *total_length, 1, length, fp);
      *total_length += length;
    } else {
      // Skip non-IDAT chunk data
      fseek(fp, length, SEEK_CUR);
    }

    // Read the CRC
    fread(&crc, 4, 1, fp);
  }

  return buffer;
}

// Function to read and check PNG signature
static int check_png_signature(FILE* fp) {
  unsigned char signature[PNG_SIGNATURE_SIZE];
  fread(signature, 1, PNG_SIGNATURE_SIZE, fp);

  unsigned char png_signature[PNG_SIGNATURE_SIZE] = {137, 80, 78, 71, 13, 10, 26, 10};
  for (int i = 0; i < PNG_SIGNATURE_SIZE; i++) {
    if (signature[i] != png_signature[i]) {
      return 0;  // Not a PNG file
    }
  }
  return 1;  // Valid PNG signature
}

// Function to determine the number of channels based on the color type
static uint32_t get_png_color_type_channels(uint8_t color_type) {
  switch (color_type) {
    case 0:
      return 1;  // Grayscale
    case 2:
      return 3;  // RGB
    case 3:
      return 3;  // Indexed-color (palette-based, typically treated as RGB)
    case 4:
      return 2;  // Grayscale with alpha
    case 6:
      return 4;  // RGBA
    default:
      return 0;  // Unknown or unsupported color type
  }
}

static void process_IHDR(const unsigned char* ihdr_data, uint32_t* tex_width, uint32_t* tex_height, uint32_t* tex_channels, uint8_t* bit_depth, uint8_t* color_type) {
  unsigned char compression_method, filter_method, interlace_method;

  // Extract width and height (each 4 bytes)
  *tex_width = (ihdr_data[0] << 24) | (ihdr_data[1] << 16) | (ihdr_data[2] << 8) | ihdr_data[3];
  *tex_height = (ihdr_data[4] << 24) | (ihdr_data[5] << 16) | (ihdr_data[6] << 8) | ihdr_data[7];

  // Extract bit depth, color type, compression method, filter method, interlace method (each 1 byte)
  *bit_depth = ihdr_data[8];
  *color_type = ihdr_data[9];
  compression_method = ihdr_data[10];
  filter_method = ihdr_data[11];
  interlace_method = ihdr_data[12];

  // Print the extracted information
  // printf("IHDR Chunk:\n");
  // printf("Width: %u\n", *tex_width);
  // printf("Height: %u\n", *tex_height);
  // printf("Bit Depth: %d\n", *bit_depth);
  // printf("Color Type: %d\n", *color_type);
  // printf("Compression Method: %d\n", compression_method);
  // printf("Filter Method: %d\n", filter_method);
  // printf("Interlace Method: %d\n", interlace_method);

  // Determine the number of channels based on the color type
  *tex_channels = get_png_color_type_channels(*color_type);
}

static int read_chunk(FILE* fp, unsigned char** idat_concatenated, unsigned int* idat_length, unsigned int* idat_buffer_size, uint32_t* tex_width, uint32_t* tex_height, uint32_t* tex_channels, uint8_t* bit_depth, uint8_t* color_type) {
  uint32_t length;
  unsigned char type[4];
  uint32_t crc;

  // Read the length of the chunk
  if (fread(&length, 4, 1, fp) != 1)
    return 1;  // Return 0 if failed to read length (possible EOF)

  length = swap_uint32(length);

  // Read the chunk type
  if (fread(type, 1, 4, fp) != 4)
    return 1;  // Return 0 if failed to read chunk type

  // Check if the chunk is an IDAT chunk
  if (strncmp((const char*)type, "IDAT", 4) == 0) {
    // Resize the buffer if needed
    if (*idat_length + length > *idat_buffer_size) {
      *idat_buffer_size = *idat_length + length;
      *idat_concatenated = realloc(*idat_concatenated, *idat_buffer_size);
      if (*idat_concatenated == NULL) {
        fprintf(stderr, "Memory reallocation failed\n");
        exit(EXIT_FAILURE);
      }
    }

    // Read IDAT chunk data
    fread(*idat_concatenated + *idat_length, 1, length, fp);
    *idat_length += length;
  } else {
    // For non-IDAT chunks
    unsigned char* data = (unsigned char*)malloc(length);
    if (data == NULL) {
      fprintf(stderr, "Memory allocation failed\n");
      exit(EXIT_FAILURE);
    }
    fread(data, 1, length, fp);

    // Process IHDR chunk
    if (strncmp((const char*)type, "IHDR", 4) == 0)
      process_IHDR(data, tex_width, tex_height, tex_channels, bit_depth, color_type);

    // Free the data buffer for non-IDAT chunks
    free(data);
  }

  // Read the CRC
  fread(&crc, 4, 1, fp);
  crc = swap_uint32(crc);

  // Print chunk information
  // printf("Chunk: %c%c%c%c, Length: %u, CRC: %u\n", type[0], type[1], type[2], type[3], length, crc);

  // Check for IEND chunk
  if (strncmp((const char*)type, "IEND", 4) == 0)
    return 1;  // Stop reading chunks if IEND is encountered

  return 0;
}

static void* texture_read_png(const char* filename, const char* asset_directory, uint32_t* tex_width, uint32_t* tex_height, uint32_t* tex_channels, uint8_t* bit_depth, uint8_t* color_type) {
  unsigned char* idat_concatenated = NULL;
  unsigned int idat_length = 0;
  unsigned int idat_buffer_size = 0;

  char complete_path[MAX_LENGTH_OF_PATH] = {0};
  snprintf(complete_path, sizeof(complete_path), "%s%s", asset_directory, filename);

  FILE* fp;
#ifdef _WIN32
  if (fopen_s(&fp, complete_path, "rb") != 0)
#else
  fp = fopen(complete_path, "rb");
  if (!fp)
#endif
  {
    perror("Error opening file");
    return NULL;
  }

  if (!check_png_signature(fp)) {
    fprintf(stderr, "File is not a valid PNG.\n");
    fclose(fp);
    return NULL;
  }

  while (!feof(fp))
    read_chunk(fp, &idat_concatenated, &idat_length, &idat_buffer_size, tex_width, tex_height, tex_channels, bit_depth, color_type);

  size_t decompressed_length = *tex_width * *tex_height * *tex_channels + *tex_height * (*bit_depth == 16 ? 2 : 1);

  unsigned char* decompressed_data = calloc(1, decompressed_length);

  if (!decompressed_data) {
    fprintf(stderr, "Failed to allocate memory for decompressed data\n");
    free(idat_concatenated);
    fclose(fp);
    return NULL;
  }

  zbuf a = {0};
  a.zbuffer = idat_concatenated;
  a.zbuffer_end = idat_concatenated + idat_length;

  if (do_zlib(&a, (char*)decompressed_data, decompressed_length, 1, 1) == 0) {
    fprintf(stderr, "Decompression failed\n");
    free(decompressed_data);
    free(idat_concatenated);
    fclose(fp);
    return NULL;
  }

  if ((size_t)(a.zout - a.zout_start) < decompressed_length) {
    log_message(LOG_SEVERITY_WARNING, "Decompressed data is smaller than expected\n");
    free(decompressed_data);
    free(idat_concatenated);
    fclose(fp);
    return NULL;
  }

  void* pixels;

  if (*bit_depth == 8)
    pixels = reverse_png_filtering(a.zout_start, *tex_width, *tex_height, *tex_channels, *color_type);
  else if (*bit_depth == 16)
    pixels = reverse_png_filtering_16bit(a.zout_start, *tex_width, *tex_height, *tex_channels * 2, *color_type);
  else {
    log_message(LOG_SEVERITY_WARNING, "Unsupported bit depth: %d\n", *bit_depth);
    free(decompressed_data);
    free(idat_concatenated);
    fclose(fp);
    return NULL;
  }

  free(a.zout_start);
  free(idat_concatenated);
  fclose(fp);

  return pixels;
}

#include "mana/graphics/utilities/texture/texture.h"

static char* build_mip_path(const char* base_path, uint32_t level) {
  // base_path is something like: "/textures/waterm1.png"

  char* out = strdup(base_path);
  if (!out) return NULL;

  // Find the extension
  char* dot = strrchr(out, '.');
  if (!dot) return out;

  // Walk backwards to find the first digit before the extension
  char* p = dot;
  while (p > out) {
    p--;
    if (*p >= '0' && *p <= '9') {
      *p = '1' + level;  // level 0 → '1', level 1 → '2', etc.
      break;
    }
  }

  return out;
}

// TODO: This needs to have missing file error check
uint8_t texture_init(struct Texture* texture, struct TextureManagerCommon* texture_manager_common, struct APICommon* api_common, struct TextureSettings texture_settings, const char* path, size_t texture_index) {
  struct TextureCommon* texture_common = &(texture->texture_common);
  texture_common->texture_settings = texture_settings;

  texture_common->path = strdup(path);

  char* name_location = strrchr(texture_common->path, '/');
#ifdef _WIN32
  if (!name_location) name_location = strrchr(texture_common->path, '\\');
#endif
  texture_common->name = name_location ? strdup(name_location + 1) : strdup(texture_common->path);

  char* type_location = strrchr(texture_common->path, '.');
  texture_common->type = type_location ? strdup(type_location + 1) : strdup("");

  texture_common->texture_manager_common = texture_manager_common;
  texture_common->id = texture_index;

  uint32_t w0 = 0, h0 = 0, ch0 = 0;
  uint8_t bit0 = 0, ct0 = 0;

  void* pixels0 = texture_read_png(texture_common->path, api_common->asset_directory, &w0, &h0, &ch0, &bit0, &ct0);

  if (!pixels0) {
    log_message(LOG_SEVERITY_WARNING, "Failed to load texture image: %s\n", texture_common->path);

    return 1;
  }

  // If ct0 == 2, pixels0 is now RGBA => force channels=4 so sizes match GPU upload.
  if (ct0 == 2) ch0 = 4;

  // Pick format from bit depth + channels
  switch (bit0) {
    case 8:
      texture_common->texture_settings.format_type = (ch0 == 1) ? FORMAT_R8_UNORM : FORMAT_R8G8B8A8_UNORM;
      break;
    case 16:
      texture_common->texture_settings.format_type = (ch0 == 1) ? FORMAT_R16_UNORM : FORMAT_R16G16B16A16_UNORM;
      break;
    default:
      log_message(LOG_SEVERITY_WARNING, "Unsupported bit depth: %d\n", bit0);
      return 1;
  }

  texture_common->width = w0;
  texture_common->height = h0;

  // build packed pixel buffer if custom mip chain
  void* upload_pixels = pixels0;
  uint32_t mip_count = 1;

  if (texture_common->texture_settings.mip_type == MIP_CUSTOM) {
    mip_count = texture_common->texture_settings.mip_count;
    if (mip_count < 1) mip_count = 1;

    uint32_t bytes_per_channel = (bit0 == 16) ? 2 : 1;
    uint32_t channels = ch0;  // after ct0==2 fix => 4

    // compute total bytes for all levels (must match exactly what Vulkan copy loop uses)
    VkDeviceSize total = 0;
    for (uint32_t level = 0; level < mip_count; level++) {
      uint32_t w = w0 >> level;
      if (!w) w = 1;
      uint32_t h = h0 >> level;
      if (!h) h = 1;
      total += (VkDeviceSize)w * (VkDeviceSize)h * (VkDeviceSize)channels * (VkDeviceSize)bytes_per_channel;
    }

    uint8_t* combined = (uint8_t*)malloc((size_t)total);
    if (!combined) {
      log_message(LOG_SEVERITY_ERROR, "Failed to alloc combined mip buffer\n");
      return 1;
    }

    // copy level 0 first
    VkDeviceSize off = 0;
    VkDeviceSize sz0 = (VkDeviceSize)w0 * (VkDeviceSize)h0 * (VkDeviceSize)channels * (VkDeviceSize)bytes_per_channel;
    memcpy(combined + off, pixels0, (size_t)sz0);
    off += sz0;
    free(pixels0);

    // load + copy levels 1..N-1
    for (uint32_t level = 1; level < mip_count; level++) {
      char* mip_path = build_mip_path(texture_common->path, level);
      if (!mip_path) {
        free(combined);
        log_message(LOG_SEVERITY_ERROR, "Failed to build mip path\n");
        return 1;
      }

      uint32_t wl = 0, hl = 0, chl = 0;
      uint8_t bitl = 0, ctl = 0;
      void* pixelsL = texture_read_png(mip_path, api_common->asset_directory, &wl, &hl, &chl, &bitl, &ctl);
      free(mip_path);

      if (!pixelsL) {
        free(combined);
        log_message(LOG_SEVERITY_ERROR, "Failed to load mip level %u\n", level);
        return 1;
      }
      if (ctl == 2) chl = 4;  // same RGB->RGBA fix

      // Validate consistency (bit depth + channels) and expected sizes
      if (bitl != bit0 || chl != channels) {
        free(pixelsL);
        free(combined);
        log_message(LOG_SEVERITY_ERROR, "Mip %u format mismatch\n", level);
        return 1;
      }

      uint32_t exp_w = w0 >> level;
      if (!exp_w) exp_w = 1;
      uint32_t exp_h = h0 >> level;
      if (!exp_h) exp_h = 1;
      if (wl != exp_w || hl != exp_h) {
        free(pixelsL);
        free(combined);
        log_message(LOG_SEVERITY_ERROR, "Mip %u size mismatch (got %ux%u expected %ux%u)\n", level, wl, hl, exp_w, exp_h);
        return 1;
      }

      VkDeviceSize sz = (VkDeviceSize)wl * (VkDeviceSize)hl * (VkDeviceSize)channels * (VkDeviceSize)bytes_per_channel;
      memcpy(combined + off, pixelsL, (size_t)sz);
      off += sz;
      free(pixelsL);
    }

    upload_pixels = combined;  // Vulkan will read all mips sequentially from this
  }

#ifdef VULKAN_API_SUPPORTED
  if (api_common->api_type == API_VULKAN) texture->texture_func = VULKAN_TEXTURE;
#endif
#ifdef DIRECTX_12_API_SUPPORTED
  if (api_common->api_type == API_DIRECTX12) texture->texture_func = directx_12_TEXTURE;
#endif

  texture->texture_func.texture_init(&(texture->texture_common), texture_common->texture_manager_common, api_common, upload_pixels);

  if (texture_common->texture_settings.mip_type == MIP_CUSTOM)
    free(upload_pixels);
  else
    // TODO: This could be bug so watch this
    free(upload_pixels);

  return 0;
}

void texture_delete(struct Texture* texture, struct APICommon* api_common) {
  texture->texture_func.texture_delete(&(texture->texture_common), api_common);

  free(texture->texture_common.path);
  free(texture->texture_common.name);
  free(texture->texture_common.type);
}
