#pragma once

#include "mana/core/graphics/shaders/shader.h"

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
  void* extra_data;
};

u8 resolve_shader_init(struct ResolveShader* resolve_shader, struct APICommon* apu_api, u32 width, u32 height, u8 supersample_scale, u8 num_textures, u8 descriptors, enum MESH_TYPE mesh_type);
void resolve_shader_delete(struct ResolveShader* resolve_shader, struct APICommon* apu_api);
