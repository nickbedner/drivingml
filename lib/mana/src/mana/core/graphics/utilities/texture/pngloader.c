#include "mana/core/graphics/utilities/texture/pngloader.h"

// Helper function for Paeth filter
internal inline u8 paeth_predictor(u8 a, u8 b, u8 c) {
  i32 p = a + b - c;
  i32 pa = abs(p - a);
  i32 pb = abs(p - b);
  i32 pc = abs(p - c);
  if (pa <= pb && pa <= pc)
    return (u8)a;
  else if (pb <= pc)
    return (u8)b;
  else
    return (u8)c;
}

// Function to reverse PNG filtering for a single scanline
internal inline void unfilter_scanline(u8* recon, const u8* scanline, const u8* prev, size_t bytewidth, size_t length, u8 filter_type) {
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
internal inline u8* reverse_png_filtering(u8* data, u32 width, u32 height, u32 bpp, u8 color_type) {
  size_t linebytes = width * bpp;

  u8* prev_line = NULL;
  u8* pixels = (u8*)calloc(1, linebytes * height);
  u8* start_pos = pixels;
  if (!pixels) {
    fprintf(stderr, "Memory allocation failed\n");
    exit(EXIT_FAILURE);
  }

  for (u32 y = 0; y < height; y++) {
    u8 filter_type = *data++;  // First byte of a scanline is the filter type

    unfilter_scanline(pixels, data, prev_line, bpp, linebytes, filter_type);

    // Set the previous line to the current one for the next iteration
    prev_line = pixels;

    // Move to the next line
    data += linebytes;
    pixels += linebytes;
  }

  pixels = start_pos;

  // if (color_type == 0) {
  //   u8 *new_pixels = calloc(1, width * height * 4);
  //   u8 *new_pixels_start_pos = new_pixels;
  //   for (u32 y = 0; y < height; y++) {
  //     for (u32 x = 0; x < width; x++) {
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
    u8* new_pixels = (u8*)calloc(1, width * height * 4);
    u8* new_pixels_start_pos = new_pixels;
    for (u32 y = 0; y < height; y++) {
      for (u32 x = 0; x < width; x++) {
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

internal inline i32 paeth_predictor_16(i32 a, i32 b, i32 c) {
  i32 p = a + b - c;
  i32 pa = abs(p - a);
  i32 pb = abs(p - b);
  i32 pc = abs(p - c);

  if (pa <= pb && pa <= pc)
    return a;

  if (pb <= pc)
    return b;

  return c;
}

// Check top byte whether to wrap or not
internal inline u16 calculate_wrap_around(u16 val1, u16 val2, u8 byte_val1, u8 byte_val2) {
  // u8 clamp_val = (byte_val1 + byte_val2) % UCHAR_MAX;
  u8 clamp_val = byte_val1 + byte_val2;
  u16 corrected_short = clamp_val * 256;
  u16 added_val = val1 + val2;
  if (added_val > corrected_short + UCHAR_MAX)
    return corrected_short + UCHAR_MAX;
  if (added_val < corrected_short)
    return corrected_short;

  return added_val;
}

// Function to reverse PNG filtering for a single scanline
internal inline void unfilter_scanline_16bit(u16* recon, const u16* scanline, const u16* prev, const u8* prev_byte_line, u8* byte_pixels, u8* byte_data, size_t shortwidth, size_t length, u8 filter_type) {
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
          recon[i] = calculate_wrap_around(scanline[i], (u16)paeth_predictor_16(0, prev[i], 0), byte_data[i], paeth_predictor(0, prev_byte_line[i], 0));
          byte_pixels[i] = byte_data[i] + paeth_predictor(0, prev_byte_line[i], 0);
        }
        for (i = shortwidth; i < length; i++) {
          recon[i] = calculate_wrap_around(scanline[i], (u16)paeth_predictor_16(recon[i - shortwidth], prev[i], prev[i - shortwidth]), byte_data[i], paeth_predictor(byte_pixels[i - shortwidth], prev_byte_line[i], prev_byte_line[i - shortwidth]));
          byte_pixels[i] = byte_data[i] + paeth_predictor(byte_pixels[i - shortwidth], prev_byte_line[i], prev_byte_line[i - shortwidth]);
        }
      } else {
        for (i = 0; i < shortwidth; i++) {
          recon[i] = scanline[i];
          byte_pixels[i] = byte_data[i];
        }
        for (i = shortwidth; i < length; i++) {
          recon[i] = calculate_wrap_around(scanline[i], (u16)paeth_predictor_16(recon[i - shortwidth], 0, 0), byte_data[i], paeth_predictor(byte_pixels[i - shortwidth], 0, 0));
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

internal inline u16* reverse_png_filtering_16bit(u8* data, u32 width, u32 height, u32 bpp, u8 color_type) {
  size_t linebytes = width * bpp;  // Bytes per line

  u16* prev_line = NULL;
  u8* prev_byte_line = NULL;
  // u16 *pixels = calloc(1, linebytes * height);
  // u16 *pixels = (u16 *)_aligned_malloc(linebytes * height, alignof(u16));
  // u16 *aligned_data = (u16 *)_aligned_malloc(linebytes * height, alignof(u16));
  // u8 *byte_pixels = (u8 *)_aligned_malloc((linebytes * height) / 2, alignof(u8));
  // u8 *byte_data = (u8 *)_aligned_malloc((linebytes * height) / 2, alignof(u8));
  u16* pixels = (u16*)malloc(linebytes * height);
  u16* aligned_data = (u16*)malloc(linebytes * height);
  u8* byte_pixels = (u8*)malloc((linebytes * height) / 2);
  u8* byte_data = (u8*)malloc((linebytes * height) / 2);
  u16* start_pos = pixels;
  u16* data_start_pos = aligned_data;
  u8* byte_pixels_start_pos = byte_pixels;
  u8* byte_data_start_pos = byte_data;

  // memset(pixels, 0, linebytes * height);
  // memset(aligned_data, 0, linebytes * height);
  // memset(byte_data, 0, (linebytes * height) / 2);
  // memset(byte_pixels, 0, (linebytes * height) / 2);

  u8* data_start = data;
  for (u32 y = 0; y < height; y++) {
    u8* pixel_data = data_start + 1;

    for (u32 i = 0; i < width * bpp / 2; i++) {
      u8 high_byte, low_byte;
      memcpy(&high_byte, pixel_data + i * 2, 1);
      memcpy(&low_byte, pixel_data + i * 2 + 1, 1);
      aligned_data[i] = (u16)((high_byte << 8) | low_byte);
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

  for (u32 y = 0; y < height; y++) {
    u8 filter_type = *data++;  // Read filter type as 8-bit value

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
    u16* new_pixels = (u16*)calloc(1, width * height * 8);
    u16* new_pixels_start_pos = new_pixels;
    for (u32 y = 0; y < height; y++) {
      for (u32 x = 0; x < width; x++) {
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
    u16* new_pixels = (u16*)calloc(1, width * height * 8);
    u16* new_pixels_start_pos = new_pixels;
    for (u32 y = 0; y < height; y++) {
      for (u32 x = 0; x < width; x++) {
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

// Manually define a simple byte-swapping function for u32
internal inline u32 swap_uint32(u32 val) {
  return ((val << 24) & 0xff000000) | ((val << 8) & 0x00ff0000) | ((val >> 8) & 0x0000ff00) | ((val >> 24) & 0x000000ff);
}

// Function to concatenate IDAT chunks
internal inline unsigned char* concatenate_idat_chunks(FILE* fp, unsigned int* total_length) {
  unsigned int buffer_size = 1024;  // Initial buffer size
  unsigned char* buffer = (unsigned char*)malloc(buffer_size);
  if (buffer == NULL) {
    fprintf(stderr, "Memory allocation failed\n");
    exit(EXIT_FAILURE);
  }

  *total_length = 0;  // Initialize total length

  // Read each chunk
  while (!feof(fp)) {
    u32 length;
    unsigned char type[4];
    u32 crc;

    // Read the length of the chunk
    fread(&length, 4, 1, fp);
    length = swap_uint32(length);

    // Read the chunk type
    fread(type, 1, 4, fp);

    if (strncmp((const char*)type, "IDAT", 4) == 0) {
      // Resize buffer if needed
      if (*total_length + length > buffer_size) {
        buffer_size = *total_length + length;
        buffer = (unsigned char*)realloc(buffer, buffer_size);
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
      fseek(fp, (long)length, SEEK_CUR);
    }

    // Read the CRC
    fread(&crc, 4, 1, fp);
  }

  return buffer;
}

// Function to determine the number of channels based on the color type
internal u32 get_png_color_type_channels(u8 color_type) {
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

internal inline void process_IHDR(const u8* ihdr_data, u32* tex_width, u32* tex_height, u32* tex_channels, u8* bit_depth, u8* color_type) {
  unsigned char compression_method, filter_method, interlace_method;

  // Extract width and height (each 4 bytes)
  *tex_width = (u32)(ihdr_data[0] << 24) | (u32)(ihdr_data[1] << 16) | (u32)(ihdr_data[2] << 8) | ihdr_data[3];
  *tex_height = (u32)(ihdr_data[4] << 24) | (u32)(ihdr_data[5] << 16) | (u32)(ihdr_data[6] << 8) | ihdr_data[7];

  // Extract bit depth, color type, compression method, filter method, interlace method (each 1 byte)
  *bit_depth = ihdr_data[8];
  *color_type = ihdr_data[9];
  compression_method = ihdr_data[10];
  filter_method = ihdr_data[11];
  interlace_method = ihdr_data[12];

  // Print the extracted information
  if (PNG_DEBUG) {
    log_message(LOG_SEVERITY_DEBUG, "IHDR Chunk:");
    log_message(LOG_SEVERITY_DEBUG, "Width: %u", *tex_width);
    log_message(LOG_SEVERITY_DEBUG, "Height: %u", *tex_height);
    log_message(LOG_SEVERITY_DEBUG, "Bit Depth: %d", *bit_depth);
    log_message(LOG_SEVERITY_DEBUG, "Color Type: %d", *color_type);
    log_message(LOG_SEVERITY_DEBUG, "Compression Method: %d", compression_method);
    log_message(LOG_SEVERITY_DEBUG, "Filter Method: %d", filter_method);
    log_message(LOG_SEVERITY_DEBUG, "Interlace Method: %d", interlace_method);
  }

  // Determine the number of channels based on the color type
  *tex_channels = get_png_color_type_channels(*color_type);
}

internal inline int read_chunk(FILE* fp, unsigned char** idat_concatenated, unsigned int* idat_length, unsigned int* idat_buffer_size, u32* tex_width, u32* tex_height, u32* tex_channels, u8* bit_depth, u8* color_type) {
  u32 length;
  unsigned char type[4];
  u32 crc;

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
      *idat_concatenated = (unsigned char*)realloc(*idat_concatenated, *idat_buffer_size);
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

internal inline int check_png_signature(FILE* fp) {
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

void* png_loader_read_png(const char* filename, const char* asset_directory, u32* tex_width, u32* tex_height, u32* tex_channels, u8* bit_depth, u8* color_type) {
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

  unsigned char* decompressed_data = (unsigned char*)calloc(1, decompressed_length);

  if (!decompressed_data) {
    fprintf(stderr, "Failed to allocate memory for decompressed data\n");
    free(idat_concatenated);
    fclose(fp);
    return NULL;
  }

  zbuf a = {0};
  a.zbuffer = idat_concatenated;
  a.zbuffer_end = idat_concatenated + idat_length;

  if (do_zlib(&a, (i8*)decompressed_data, decompressed_length, 1, 1) == 0) {
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
    pixels = reverse_png_filtering((u8*)a.zout_start, *tex_width, *tex_height, *tex_channels, *color_type);
  else if (*bit_depth == 16)
    pixels = reverse_png_filtering_16bit((u8*)a.zout_start, *tex_width, *tex_height, *tex_channels * 2, *color_type);
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
