#pragma once

#include "mana/graphics/apis/api.h"
#ifdef VULKAN_API_SUPPORTED
#include "mana/graphics/entities/sprite/spriteanimationvulkan.h"
#endif
#ifdef DIRECTX_12_API_SUPPORTED
#include "mana/graphics/entities/sprite/spriteanimationdirectx12.h"
#endif

struct SpriteAnimationFunc {
  uint_fast8_t (*sprite_animation_init)(struct SpriteAnimationCommon *, struct APICommon *, struct Shader *, struct Texture *);
  void (*sprite_animation_delete)(struct SpriteAnimationCommon *, struct APICommon *);
  void (*sprite_animation_render)(struct SpriteAnimationCommon *, struct GBufferCommon *);
  void (*sprite_animation_update_uniforms)(struct SpriteAnimationCommon *, struct APICommon *, struct GBufferCommon *gbuffer_common);
};

#ifdef VULKAN_API_SUPPORTED
static const struct SpriteAnimationFunc VULKAN_SPRITE_ANIMATION = {sprite_animation_vulkan_init, sprite_animation_vulkan_delete, sprite_animation_vulkan_render, sprite_animation_vulkan_update_uniforms};
#endif
#ifdef DIRECTX_12_API_SUPPORTED
static const struct SpriteAnimationFunc DIRECTX_12_SPRITE_ANIMATION = {sprite_animation_directx_12_init, sprite_animation_directx_12_delete, sprite_animation_directx_12_render, sprite_animation_directx_12_update_uniforms};
#endif

struct SpriteAnimation {
  struct SpriteAnimationFunc sprite_animation_func;
  struct SpriteAnimationCommon sprite_animation_common;
};

uint_fast8_t sprite_animation_init(struct SpriteAnimation *sprite_animation, struct APICommon *api_common, struct Shader *shader, struct Texture *texture, size_t num, uint8_t frames, float frame_length, float padding);
void sprite_animation_delete(struct SpriteAnimation *sprite_animation, struct APICommon *api_common);
void sprite_animation_render(struct SpriteAnimation *sprite_animation, struct GBufferCommon *gbuffer_common);
void sprite_animation_update_uniforms(struct SpriteAnimation *sprite_animation, struct APICommon *api_common, struct GBufferCommon *gbuffer_common);
void sprite_animation_recreate(struct SpriteAnimation *sprite_animation, struct APICommon *api_common, size_t num, uint8_t frames, float frame_length, uint8_t padding);
void sprite_animation_set_frame(struct SpriteAnimation *sprite_animation, int frame);
void sprite_animation_update(struct SpriteAnimation *sprite_animation, double delta_time);
