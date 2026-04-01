#pragma once

#include "mana/core/graphics/apis/api.h"

#ifdef VULKAN_API_SUPPORTED
#include "mana/core/graphics/entities/water/watervulkan.h"
#endif
#ifdef DIRECTX_12_API_SUPPORTED
#include "mana/core/graphics/entities/water/waterdirectx12.h"
#endif

#include "mana/core/graphics/entities/water/watercommon.h"

struct WaterFunc {
  u8 (*water_init)(struct WaterCommon*, struct APICommon*, struct Shader*, struct Texture*);
  void (*water_delete)(struct WaterCommon*, struct APICommon*);
  void (*water_render)(struct WaterCommon*, struct GBufferCommon*);
  void (*water_update_uniforms)(struct WaterCommon*, struct APICommon*, struct GBufferCommon*, u32, u32);
};

#ifdef VULKAN_API_SUPPORTED
global const struct WaterFunc VULKAN_WATER = {water_vulkan_init, water_vulkan_delete, water_vulkan_render, water_vulkan_update_uniforms};
#endif
#ifdef DIRECTX_12_API_SUPPORTED
global const struct WaterFunc DIRECTX_12_WATER = {water_directx_12_init, water_directx_12_delete, water_directx_12_render, water_directx_12_update_uniforms};
#endif

struct Water {
  struct WaterFunc water_func;
  struct WaterCommon water_common;
};

u8 water_init(struct Water*, struct APICommon*, struct Shader*, struct Texture*);
void water_delete(struct Water*, struct APICommon*);
void water_render(struct Water*, struct GBufferCommon*);
void water_update_uniforms(struct Water*, struct APICommon*, struct GBufferCommon*, u32 width, u32 height);
