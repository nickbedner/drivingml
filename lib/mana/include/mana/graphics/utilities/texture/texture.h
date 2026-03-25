#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "mana/graphics/apis/api.h"
#include "mana/graphics/utilities/texture/pngloader.h"
#include "mana/utilities/zlib.h"

#ifdef VULKAN_API_SUPPORTED
#include "mana/graphics/utilities/texture/texturevulkan.h"
#endif
#ifdef DIRECTX_12_API_SUPPORTED
#include "mana/graphics/utilities/texture/texturedirectx12.h"
#endif

struct TextureFunc {
  uint_fast8_t (*texture_init)(struct TextureCommon*, struct TextureManagerCommon*, struct APICommon*, void*);
  void (*texture_delete)(struct TextureCommon*, struct APICommon*);
};

#ifdef VULKAN_API_SUPPORTED
static const struct TextureFunc VULKAN_TEXTURE = {texture_vulkan_init, texture_vulkan_delete};
#endif
#ifdef DIRECTX_12_API_SUPPORTED
static const struct TextureFunc directx_12_TEXTURE = {texture_directx_12_init, texture_directx_12_delete};
#endif

struct Texture {
  struct TextureFunc texture_func;
  struct TextureCommon texture_common;
};

uint8_t texture_init(struct Texture* texture, struct TextureManagerCommon* texture_manager_common, struct APICommon* api_common, struct TextureSettings texture_settings, const char* path, size_t texture_index, bool is_sprite);
uint8_t texture_array_init(struct Texture* texture, struct TextureManagerCommon* texture_manager_common, struct APICommon* api_common, struct TextureSettings texture_settings, const char* const* paths, uint32_t layer_count, size_t texture_index);
void texture_delete(struct Texture* texture, struct APICommon* api_common);
