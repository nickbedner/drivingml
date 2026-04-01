#pragma once

#include "mana/core/graphics/shaders/shader.h"

// Effect for blitting images

struct BlitUniformBufferObject {
  alignas(16) vec2 screen_size;
};

struct BlitShader {
  struct Shader shader;
  void* extra_data;
};

u8 blit_shader_init(struct BlitShader* blit_effect, struct APICommon* apu_api, u32 width, u32 height, u8 supersample_scale, u8 num_textures, u8 descriptors, enum MESH_TYPE mesh_type);
void blit_shader_delete(struct BlitShader* blit_effect, struct APICommon* apu_api);
