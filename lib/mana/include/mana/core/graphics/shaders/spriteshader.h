#pragma once

#include "mana/core/graphics/render/gbuffer/gbuffercommon.h"
#include "mana/core/graphics/render/swapchain/swapchain.h"
#include "mana/core/graphics/shaders/shader.h"
#include "mana/core/graphics/utilities/mesh/mesh.h"

// Effect for blitting sprite to gbuffer

#define SPRITE_SHADER_COLOR_ATTACHEMENTS 2
#define SPRITE_SHADER_VERTEX_ATTRIBUTES 2

struct SpriteShader {
  struct Shader shader;
  void* extra_data;
};

u8 sprite_shader_init(struct SpriteShader* sprite_shader, struct APICommon* api_common, u32 width, u32 height, u8 supersample_scale, struct GBufferCommon* gbuffer_common, b8 depth_test, b8 depth_write, const uint_fast32_t msaa_samples, uint_fast32_t descriptors);
void sprite_shader_delete(struct SpriteShader* sprite_shader, struct APICommon* api_common);
