#pragma once

#include "mana/graphics/render/gbuffer/gbuffercommon.h"
#include "mana/graphics/render/swapchain/swapchain.h"
#include "mana/graphics/shaders/shader.h"
#include "mana/graphics/utilities/mesh/mesh.h"

// Effect for blitting sprite animations to gbuffer

#define SPRITE_ANIMATION_SHADER_COLOR_ATTACHEMENTS 2
#define SPRITE_ANIMATION_SHADER_VERTEX_ATTRIBUTES 2

struct SpriteAnimationShader {
  struct Shader shader;
  void *extra_data;
};

int sprite_animation_shader_init(struct SpriteAnimationShader *sprite_animation_shader, struct APICommon *api_common, uint32_t width, uint32_t height, uint_fast8_t supersample_scale, struct GBufferCommon *gbuffer_common, bool depth_test, const uint_fast32_t msaa_samples, uint_fast32_t descriptors);
void sprite_animation_shader_delete(struct SpriteAnimationShader *sprite_animation_shader, struct APICommon *api_common);
