#include "mana/graphics/shaders/watershader.h"

u8 water_shader_init(struct WaterShader* water_shader, struct APICommon* api_common, u32 width, u32 height, u8 supersample_scale, struct GBufferCommon* gbuffer_common, const uint_fast32_t msaa_samples, uint_fast32_t descriptors) {
  struct ShaderSettings* shader_settings = &(water_shader->shader.shader_common.shader_settings);
  shader_settings->vertex_shader = "water";
  shader_settings->fragment_shader = "water";
  shader_settings->front_face = SHADER_FRONT_FACE_COUNTER_CLOCKWISE;
  shader_settings->cull_mode = SHADER_CULL_MODE_BACK_BIT;
  shader_settings->depth_test = TRUE;
  shader_settings->depth_write = FALSE;
  shader_settings->supersampled = TRUE;
  shader_settings->num_msaa_samples = msaa_samples;
  shader_settings->color_attachments = WATER_SHADER_COLOR_ATTACHEMENTS;
  shader_settings->vertex_attributes = WATER_SHADER_VERTEX_ATTRIBUTES;
  shader_settings->blend = TRUE;
  shader_settings->mesh_type = MESH_TYPE_QUAD;
  shader_settings->mesh_memory_size = mesh_get_memory_size(shader_settings->mesh_type);
  shader_settings->uniforms_constants = 2;
  shader_settings->uniform_constant_state[0] = (struct ShaderAttributeSettings){.shader_stage = SHADER_STAGE_VERTEX, .shader_position = 0};
  shader_settings->uniform_constant_state[1] = (struct ShaderAttributeSettings){.shader_stage = SHADER_STAGE_FRAGMENT, .shader_position = 1};
  shader_settings->texture_samples = 1;
  shader_settings->texture_sample_state[0] = (struct ShaderAttributeSettings){.shader_stage = SHADER_STAGE_FRAGMENT, .shader_position = 2};
  shader_settings->render_targets = 3;
  shader_settings->render_target_format[0] = SHADER_RENDER_TARGET_FORMAT_R8G8B8A8_UNORM;
  shader_settings->render_target_format[1] = SHADER_RENDER_TARGET_FORMAT_R8G8B8A8_UNORM;
  shader_settings->render_target_format[2] = SHADER_RENDER_TARGET_FORMAT_D32_FLOAT;
  shader_settings->descriptors = descriptors;
// Note: This is a hack to get the render pass to the shader for vulkan
#ifdef VULKAN_API_SUPPORTED
  if (api_common->api_type == API_VULKAN)
    shader_settings->extra_data = gbuffer_common->gbuffer_vulkan.render_pass;
#endif

  shader_init(&(water_shader->shader), api_common, width, height, supersample_scale);

  return 0;
}

void water_shader_delete(struct WaterShader* water_shader, struct APICommon* api_common) {
  shader_delete(&water_shader->shader, api_common);
}
