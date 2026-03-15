#include "mana/graphics/shaders/resolveshader.h"

uint_fast8_t resolve_shader_init(struct ResolveShader* resolve_shader, struct APICommon* api_common, uint32_t width, uint32_t height, uint_fast8_t supersample_scale, uint_fast8_t num_textures, uint_fast8_t descriptors, enum MESH_TYPE mesh_type) {
  struct ShaderSettings common_settings;
  common_settings.vertex_shader = "screenspace";
  common_settings.front_face = SHADER_FRONT_FACE_COUNTER_CLOCKWISE;
  common_settings.cull_mode = SHADER_CULL_MODE_NONE;
  common_settings.depth_test = false;
  common_settings.supersampled = false;
  common_settings.num_msaa_samples = 1;
  common_settings.color_attachments = 1;
  common_settings.vertex_attributes = 0;  // Note: Vertices are setup in the vertex shader
  common_settings.blend = false;
  common_settings.mesh_type = mesh_type;
  common_settings.mesh_memory_size = mesh_get_memory_size(common_settings.mesh_type);
  common_settings.uniforms_constants = 1;
  common_settings.uniform_constant_state[0] = (struct ShaderAttributeSettings){.shader_stage = SHADER_STAGE_FRAGMENT, .shader_position = 0};
  common_settings.texture_samples = 1;
  common_settings.texture_sample_state[0] = (struct ShaderAttributeSettings){.shader_stage = SHADER_STAGE_FRAGMENT, .shader_position = 1};
  common_settings.render_targets = 1;
  common_settings.render_target_format[0] = SHADER_RENDER_TARGET_FORMAT_R8G8B8A8_UNORM;
  common_settings.descriptors = descriptors;
  common_settings.extra_data = resolve_shader->extra_data;

  struct ShaderSettings* resolve1_settings = &(resolve_shader->shader[0].shader_common.shader_settings);
  *resolve1_settings = common_settings;
  resolve1_settings->fragment_shader = "blit";
  shader_init((&resolve_shader->shader[0]), api_common, width, height, supersample_scale);

  struct ShaderSettings* resolve2_settings = &(resolve_shader->shader[1].shader_common.shader_settings);
  *resolve2_settings = common_settings;
  resolve2_settings->fragment_shader = "resolve/resolve2";
  shader_init((&resolve_shader->shader[1]), api_common, width, height, supersample_scale);

  struct ShaderSettings* resolve3_settings = &(resolve_shader->shader[2].shader_common.shader_settings);
  *resolve3_settings = common_settings;
  resolve3_settings->fragment_shader = "resolve/resolve3";
  shader_init((&resolve_shader->shader[2]), api_common, width, height, supersample_scale);

  struct ShaderSettings* resolve4_settings = &(resolve_shader->shader[3].shader_common.shader_settings);
  *resolve4_settings = common_settings;
  resolve4_settings->fragment_shader = "resolve/resolve4";
  shader_init((&resolve_shader->shader[3]), api_common, width, height, supersample_scale);

  return 0;
}

void resolve_shader_delete(struct ResolveShader* resolve_shader, struct APICommon* api_common) {
  for (uint_fast8_t shader_num = 0; shader_num < 4; shader_num++)
    shader_delete(&(resolve_shader->shader[shader_num]), api_common);
}
