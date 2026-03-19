#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "mana/graphics/apis/api.h"
#include "mana/utilities/zlib.h"

#define PNG_SIGNATURE_SIZE 8
static const bool PNG_DEBUG = false;

void* png_loader_read_png(const char* filename, const char* asset_directory, uint32_t* tex_width, uint32_t* tex_height, uint32_t* tex_channels, uint8_t* bit_depth, uint8_t* color_type);
