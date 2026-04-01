#pragma once

#include "mana/core/graphics/apis/api.h"
#ifdef VULKAN_API_SUPPORTED
#include "mana/core/graphics/entities/model/modelvulkan.h"
#endif
#ifdef DIRECTX_12_API_SUPPORTED
#include "mana/core/graphics/entities/model/modeldirectx12.h"
#endif

struct ModelFunc {
  void (*model_clone_init)(struct ModelCommon*, struct APICommon*);
  void (*model_clone_delete)(struct ModelCommon*, struct APICommon*);
  void (*model_render)(struct ModelCommon*, struct GBuffer*, r64);
  void (*model_update_uniforms)(struct ModelCommon*, struct APICommon*, struct GBuffer*, vec3d, vec4, vec4, vec4, vec4);
};

#ifdef VULKAN_API_SUPPORTED
global const struct ModelFunc VULKAN_MODEL = {model_vulkan_clone_init, model_vulkan_clone_delete, model_vulkan_render, model_vulkan_update_uniforms};
#endif
#ifdef DIRECTX_12_API_SUPPORTED
global const struct ModelFunc directx_12_MODEL = {model_directx_12_clone_init, model_directx_12_clone_delete, model_directx_12_render, model_directx_12_update_uniforms};
#endif

struct Model {
  struct ModelFunc model_func;
  struct ModelCommon model_common;
};

u8 model_init(struct Model* model, struct APICommon* api_common, struct ModelSettings* model_settings, size_t num);
void model_delete(struct Model* model, struct APICommon* api_common);
struct Model* model_get_clone(struct Model* model, struct APICommon* api_common);

void model_clone_delete(struct Model* model, struct APICommon* api_common);
void model_render(struct Model* model, struct GBuffer* gbuffer, r64 delta_time);
void model_update_uniforms(struct Model* model, struct APICommon* api_common, struct GBuffer* gbuffer, vec3d position, vec4 light_pos, vec4 diffuse_color, vec4 ambient_color, vec4 specular_light);
void model_recreate(struct Model* model, struct APICommon* api_common);
