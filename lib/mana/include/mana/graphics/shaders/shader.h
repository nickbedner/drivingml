#pragma once

#include "mana/graphics/apis/api.h"
#ifdef VULKAN_API_SUPPORTED
#include "mana/graphics/shaders/shadervulkan.h"
#endif
#ifdef DIRECTX_12_API_SUPPORTED
#include "mana/graphics/shaders/shaderdirectx12.h"
#endif

struct ShaderFunc {
  uint_fast8_t (*shader_init)(struct ShaderCommon *, struct APICommon *, uint32_t, uint32_t, uint_fast8_t);
  uint_fast8_t (*shader_compute_init)(struct ShaderCommon *, struct APICommon *);
  void (*shader_delete)(struct ShaderCommon *, struct APICommon *);
  void (*shader_resize)(struct ShaderCommon *, struct APICommon *, uint32_t, uint32_t, uint_fast8_t);
};

#ifdef VULKAN_API_SUPPORTED
static const struct ShaderFunc VULKAN_SHADER = {shader_vulkan_init, shader_compute_vulkan_init, shader_vulkan_delete, shader_vulkan_resize};
#endif
#ifdef DIRECTX_12_API_SUPPORTED
static const struct ShaderFunc directx_12_SHADER = {shader_directx_12_init, shader_compute_directx_12_init, shader_directx_12_delete, shader_directx_12_resize};
#endif

struct Shader {
  struct ShaderFunc shader_func;
  struct ShaderCommon shader_common;
};

// TODO: Create struct for settings to reduce parameters
uint_fast8_t shader_init(struct Shader *shader, struct APICommon *api_common, uint32_t width, uint32_t height, uint_fast8_t supersample_scale);
uint_fast8_t shader_compute_init(struct Shader *shader, struct APICommon *api_common);
void shader_delete(struct Shader *shader, struct APICommon *api_common);
void shader_resize(struct Shader *shader, struct APICommon *api_common, uint32_t width, uint32_t height, uint_fast8_t supersample_scale);
