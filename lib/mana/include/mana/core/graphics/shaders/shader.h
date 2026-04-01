#pragma once

#include "mana/core/graphics/apis/api.h"
#ifdef VULKAN_API_SUPPORTED
#include "mana/core/graphics/shaders/shadervulkan.h"
#endif
#ifdef DIRECTX_12_API_SUPPORTED
#include "mana/core/graphics/shaders/shaderdirectx12.h"
#endif

struct ShaderFunc {
  u8 (*shader_init)(struct ShaderCommon*, struct APICommon*, u32, u32, u8);
  u8 (*shader_compute_init)(struct ShaderCommon*, struct APICommon*);
  void (*shader_delete)(struct ShaderCommon*, struct APICommon*);
  void (*shader_resize)(struct ShaderCommon*, struct APICommon*, u32, u32, u8);
};

#ifdef VULKAN_API_SUPPORTED
global const struct ShaderFunc VULKAN_SHADER = {shader_vulkan_init, shader_compute_vulkan_init, shader_vulkan_delete, shader_vulkan_resize};
#endif
#ifdef DIRECTX_12_API_SUPPORTED
global const struct ShaderFunc directx_12_SHADER = {shader_directx_12_init, shader_compute_directx_12_init, shader_directx_12_delete, shader_directx_12_resize};
#endif

struct Shader {
  struct ShaderFunc shader_func;
  struct ShaderCommon shader_common;
};

// TODO: Create struct for settings to reduce parameters
u8 shader_init(struct Shader* shader, struct APICommon* api_common, u32 width, u32 height, u8 supersample_scale);
u8 shader_compute_init(struct Shader* shader, struct APICommon* api_common);
void shader_delete(struct Shader* shader, struct APICommon* api_common);
void shader_resize(struct Shader* shader, struct APICommon* api_common, u32 width, u32 height, u8 supersample_scale);
