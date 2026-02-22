#pragma once

#include "mana/graphics/render/gbuffer/gbuffercommon.h"
#include "mana/graphics/render/swapchain/swapchain.h"
#include "mana/graphics/shaders/shader.h"
#include "mana/graphics/utilities/mesh/mesh.h"

// Effect for blitting sprite to gbuffer

#define SPRITE_SHADER_COLOR_ATTACHEMENTS 2
#define SPRITE_SHADER_VERTEX_ATTRIBUTES 2

struct SpriteShader {
  struct Shader shader;
  void *extra_data;
};

uint_fast8_t sprite_shader_init(struct SpriteShader *sprite_shader, struct APICommon *api_common, uint32_t width, uint32_t height, uint_fast8_t supersample_scale, struct GBufferCommon *gbuffer_common, bool depth_test, const uint_fast32_t msaa_samples, uint_fast32_t descriptors);
void sprite_shader_delete(struct SpriteShader *sprite_shader, struct APICommon *api_common);
