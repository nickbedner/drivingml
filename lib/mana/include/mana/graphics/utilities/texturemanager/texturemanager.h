#pragma once

#include "mana/graphics/apis/api.h"
#ifdef VULKAN_API_SUPPORTED
#include "mana/graphics/utilities/texturemanager/texturemanagervulkan.h"
#endif
#ifdef DIRECTX_12_API_SUPPORTED
#include "mana/graphics/utilities/texturemanager/texturemanagerdirectx12.h"
#endif

struct TextureManagerFunc {
  uint_fast8_t (*texture_manager_init)(struct TextureManagerCommon* texture_manager, struct APICommon*);
  void (*texture_manager_delete)(struct TextureManagerCommon* texture_manager, struct APICommon* api_common);
  uint_fast8_t (*texture_manager_add)(struct TextureManagerCommon* texture_manager, struct APICommon* api_common, struct TextureSettings texture_settings);
};

#ifdef VULKAN_API_SUPPORTED
static const struct TextureManagerFunc VULKAN_TEXTURE_MANAGER = {texture_manager_vulkan_init, texture_manager_vulkan_delete, texture_manager_vulkan_add};
#endif
#ifdef DIRECTX_12_API_SUPPORTED
static const struct TextureManagerFunc directx_12_TEXTURE_MANAGER = {texture_manager_directx_12_init, texture_manager_directx_12_delete, texture_manager_directx_12_add};
#endif

struct TextureManager {
  struct TextureManagerFunc texture_manager_func;
  struct TextureManagerCommon texture_manager_common;
};

void texture_manager_init(struct TextureManager* texture_manager, struct APICommon* api_common);
void texture_manager_delete(struct TextureManager* texture_manager, struct APICommon* api_common);
void texture_manager_add(struct TextureManager* texture_manager, struct APICommon* api_common, struct TextureSettings texture_settings, const char* path);
struct Texture* texture_manager_get(struct TextureManager* texture_manager, const char* texture_name);
