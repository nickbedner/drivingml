#include "mana/graphics/shaders/blitshader.h"

u8 blit_shader_init(struct BlitShader* blit_shader, struct APICommon* api_common, u32 width, u32 height, u8 supersample_scale, u8 num_textures, u8 descriptors, enum MESH_TYPE mesh_type) {
  struct ShaderSettings* shader_settings = &(blit_shader->shader.shader_common.shader_settings);
  shader_settings->vertex_shader = "screenspace";
  shader_settings->fragment_shader = "blit";
  shader_settings->front_face = SHADER_FRONT_FACE_COUNTER_CLOCKWISE;
  shader_settings->cull_mode = SHADER_CULL_MODE_NONE;
  shader_settings->depth_test = FALSE;
  shader_settings->supersampled = FALSE;
  shader_settings->num_msaa_samples = 1;
  shader_settings->color_attachments = 1;
  shader_settings->vertex_attributes = 0;  // Note: Vertices are setup in the vertex shader
  shader_settings->blend = FALSE;
  shader_settings->mesh_type = mesh_type;
  shader_settings->mesh_memory_size = mesh_get_memory_size(shader_settings->mesh_type);
  shader_settings->uniforms_constants = 1;
  shader_settings->uniform_constant_state[0] = (struct ShaderAttributeSettings){.shader_stage = SHADER_STAGE_FRAGMENT, .shader_position = 0};
  shader_settings->texture_samples = 1;
  shader_settings->texture_sample_state[0] = (struct ShaderAttributeSettings){.shader_stage = SHADER_STAGE_FRAGMENT, .shader_position = 1};
  shader_settings->render_targets = 1;
  shader_settings->render_target_format[0] = SHADER_RENDER_TARGET_FORMAT_R8G8B8A8_UNORM;
  shader_settings->descriptors = descriptors;
  shader_settings->extra_data = blit_shader->extra_data;

  shader_init((&blit_shader->shader), api_common, width, height, supersample_scale);

  return 0;
}

void blit_shader_delete(struct BlitShader* blit_shader, struct APICommon* api_common) {
  shader_delete(&(blit_shader->shader), api_common);
}
