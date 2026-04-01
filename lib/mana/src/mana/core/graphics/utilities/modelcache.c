#include "mana/core/graphics/utilities/modelcache.h"

void model_cache_init(struct ModelCache* model_cache, struct APICommon* api_common, u32 width, u32 height, u8 supersample_scale, struct GBufferCommon* gbuffer_common, u8 msaa_samples, uint_fast32_t descriptors) {
  // Note: Store as references because it would be dangerous to realloc in linear memory
  map_init(&model_cache->models, sizeof(struct Model*));

  model_shader_init(&(model_cache->model_shader), api_common, width, height, supersample_scale, gbuffer_common, TRUE, TRUE, msaa_samples, descriptors);
  model_static_shader_init(&(model_cache->model_static_shader), api_common, width, height, supersample_scale, gbuffer_common, TRUE, TRUE, msaa_samples, descriptors);

  model_cache->model_descriptor_set = (VkDescriptorSet*)calloc(descriptors, sizeof(VkDescriptorSet));
  vulkan_graphics_utils_create_descriptors(&(api_common->vulkan_api), model_cache->model_descriptor_set, &(model_cache->model_shader.shader.shader_common.shader_vulkan.descriptor_set_layout), &(model_cache->model_shader.shader.shader_common.shader_vulkan.descriptor_pool), model_cache->model_shader.shader.shader_common.shader_settings.descriptors);
  model_cache->model_static_descriptor_set = (VkDescriptorSet*)calloc(descriptors, sizeof(VkDescriptorSet));
  vulkan_graphics_utils_create_descriptors(&(api_common->vulkan_api), model_cache->model_static_descriptor_set, &(model_cache->model_static_shader.shader.shader_common.shader_vulkan.descriptor_set_layout), &(model_cache->model_static_shader.shader.shader_common.shader_vulkan.descriptor_pool), model_cache->model_static_shader.shader.shader_common.shader_settings.descriptors);
}

void model_cache_delete(struct ModelCache* model_cache, struct APICommon* api_common) {
  const char* model_key;
  struct MapIter model_iter = map_iter();
  while ((model_key = map_next(&model_cache->models, &model_iter))) {
    struct Model* model = *(struct Model**)map_get(&model_cache->models, model_key);
    model_delete(model, api_common);
    free(model);
  }

  map_delete(&model_cache->models);
}

void model_cache_add(struct ModelCache* model_cache, struct APICommon* api_common, struct ModelSettings* model_settings, size_t num, b8 animated) {
  struct Model* model = (struct Model*)malloc(sizeof(struct Model));

  if (animated)
    model->model_common.model_vulkan.descriptor_set = &(model_cache->model_descriptor_set[num]);
  else
    model->model_common.model_vulkan.descriptor_set = &(model_cache->model_static_descriptor_set[num]);

  model_init(model, api_common, model_settings, num);
  map_set(&(model_cache->models), model->model_common.path, &model);
}

struct Model* model_cache_get(struct ModelCache* model_cache, struct APICommon* api_common, char* model_name) {
  return model_get_clone(*((struct Model**)map_get(&(model_cache->models), model_name)), api_common);
}
