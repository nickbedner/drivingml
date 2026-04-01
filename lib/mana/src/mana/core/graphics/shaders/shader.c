#include "mana/core/graphics/shaders/shader.h"

u8 shader_init(struct Shader* shader, struct APICommon* api_common, u32 width, u32 height, u8 supersample_scale) {
#ifdef VULKAN_API_SUPPORTED
  if (api_common->api_type == API_VULKAN)
    shader->shader_func = VULKAN_SHADER;
#endif
#ifdef DIRECTX_12_API_SUPPORTED
  if (api_common->api_type == API_DIRECTX_12)
    shader->shader_func = directx_12_SHADER;
#endif

  shader->shader_func.shader_init(&(shader->shader_common), api_common, width, height, supersample_scale);

  return 0;
}

u8 shader_compute_init(struct Shader* shader, struct APICommon* api_common) {
#ifdef VULKAN_API_SUPPORTED
  if (api_common->api_type == API_VULKAN)
    shader->shader_func = VULKAN_SHADER;
#endif
#ifdef DIRECTX_12_API_SUPPORTED
  if (api_common->api_type == API_DIRECTX_12)
    shader->shader_func = directx_12_SHADER;
#endif

  shader->shader_func.shader_compute_init(&(shader->shader_common), api_common);

  return 0;
}

void shader_delete(struct Shader* shader, struct APICommon* api_common) {
  shader->shader_func.shader_delete(&(shader->shader_common), api_common);
}

void shader_resize(struct Shader* shader, struct APICommon* api_common, u32 width, u32 height, u8 supersample_scale) {
  shader->shader_func.shader_resize(&(shader->shader_common), api_common, width, height, supersample_scale);
}
