#include "mana/graphics/shaders/modelstaticshader.h"

uint_fast8_t model_static_shader_init(struct ModelStaticShader *model_static_shader, struct APICommon *api_common, uint32_t width, uint32_t height, uint_fast8_t supersample_scale, struct GBufferCommon *gbuffer_common, bool depth_test, const uint_fast32_t msaa_samples, uint_fast32_t descriptors) {
  struct ShaderSettings *shader_settings = &(model_static_shader->shader.shader_common.shader_settings);
  shader_settings->vertex_shader = L"modelstatic";
  shader_settings->fragment_shader = L"model";
  shader_settings->front_face = SHADER_FRONT_FACE_COUNTER_CLOCKWISE;
  shader_settings->cull_mode = SHADER_CULL_MODE_BACK_BIT;  // SHADER_CULL_MODE_NONE;
  shader_settings->depth_test = depth_test;
  shader_settings->supersampled = false;
  shader_settings->num_msaa_samples = msaa_samples;
  shader_settings->color_attachments = MODEL_STATIC_SHADER_COLOR_ATTACHEMENTS;
  shader_settings->vertex_attributes = MODEL_STATIC_SHADER_VERTEX_ATTRIBUTES;
  shader_settings->blend = false;
  shader_settings->mesh_type = MESH_TYPE_MODEL_STATIC;
  shader_settings->mesh_memory_size = mesh_get_memory_size(shader_settings->mesh_type);
  shader_settings->uniforms_constants = 2;
  shader_settings->uniform_constant_state[0] = (struct ShaderAttributeSettings){.shader_stage = SHADER_STAGE_VERTEX_AND_FRAGMENT, .shader_position = 0};
  shader_settings->uniform_constant_state[1] = (struct ShaderAttributeSettings){.shader_stage = SHADER_STAGE_VERTEX_AND_FRAGMENT, .shader_position = 1};
  shader_settings->texture_samples = 5;
  shader_settings->texture_sample_state[0] = (struct ShaderAttributeSettings){.shader_stage = SHADER_STAGE_FRAGMENT, .shader_position = 2};
  shader_settings->texture_sample_state[1] = (struct ShaderAttributeSettings){.shader_stage = SHADER_STAGE_FRAGMENT, .shader_position = 3};
  shader_settings->texture_sample_state[2] = (struct ShaderAttributeSettings){.shader_stage = SHADER_STAGE_FRAGMENT, .shader_position = 4};
  shader_settings->texture_sample_state[3] = (struct ShaderAttributeSettings){.shader_stage = SHADER_STAGE_FRAGMENT, .shader_position = 5};
  shader_settings->texture_sample_state[4] = (struct ShaderAttributeSettings){.shader_stage = SHADER_STAGE_FRAGMENT, .shader_position = 6};
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

  shader_init(&(model_static_shader->shader), api_common, width, height, supersample_scale);

  return 0;
}

void model_static_shader_delete(struct ModelStaticShader *model_static_shader, struct APICommon *api_common) {
  shader_delete(&model_static_shader->shader, api_common);

  vkDestroyDescriptorPool(api_common->vulkan_api.device, model_static_shader->shader.shader_common.shader_vulkan.descriptor_pool, NULL);
}
