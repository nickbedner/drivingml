#pragma once

#include "mana/graphics/shaders/shader.h"

// Effect for blitting images

struct BlitUniformBufferObject {
  alignas(16) vec2 screen_size;
};

struct BlitShader {
  struct Shader shader;
  void *extra_data;
};

uint_fast8_t blit_shader_init(struct BlitShader *blit_effect, struct APICommon *apu_api, uint32_t width, uint32_t height, uint_fast8_t supersample_scale, uint_fast8_t num_textures, uint_fast8_t descriptors, enum MESH_TYPE mesh_type);
void blit_shader_delete(struct BlitShader *blit_effect, struct APICommon *apu_api);
