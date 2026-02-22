#pragma once

#include "mana/graphics/shaders/shader.h"

// Effect for blitting images

struct ResolveUniformBufferObject {
  alignas(16) vec2 screen_size;
};

struct ResolveShader {
  // 0 is blit 1x
  // 1 is resolve 2x
  // 2 is resolve 3x
  // 3 is resolve 4x
  struct Shader shader[4];
  void *extra_data;
};

uint_fast8_t resolve_shader_init(struct ResolveShader *resolve_shader, struct APICommon *apu_api, uint32_t width, uint32_t height, uint_fast8_t supersample_scale, uint_fast8_t num_textures, uint_fast8_t descriptors, enum MESH_TYPE mesh_type);
void resolve_shader_delete(struct ResolveShader *resolve_shader, struct APICommon *apu_api);
