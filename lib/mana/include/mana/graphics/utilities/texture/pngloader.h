#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "mana/graphics/apis/api.h"
#include "mana/utilities/zlib.h"

#define PNG_SIGNATURE_SIZE 8
global const b8 PNG_DEBUG = FALSE;

void* png_loader_read_png(const char* filename, const char* asset_directory, u32* tex_width, u32* tex_height, u32* tex_channels, u8* bit_depth, u8* color_type);
