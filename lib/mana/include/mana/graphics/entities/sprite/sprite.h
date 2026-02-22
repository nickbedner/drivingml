#pragma once

#include "mana/graphics/apis/api.h"
#ifdef VULKAN_API_SUPPORTED
#include "mana/graphics/entities/sprite/spritevulkan.h"
#endif
#ifdef DIRECTX_12_API_SUPPORTED
#include "mana/graphics/entities/sprite/spritedirectx12.h"
#endif

struct SpriteFunc {
  uint_fast8_t (*sprite_init)(struct SpriteCommon *, struct APICommon *, struct Shader *, struct Texture *, size_t);
  void (*sprite_delete)(struct SpriteCommon *, struct APICommon *);
  void (*sprite_render)(struct SpriteCommon *, struct GBufferCommon *);
  void (*sprite_update_uniforms)(struct SpriteCommon *, struct APICommon *, struct GBufferCommon *);
};

#ifdef VULKAN_API_SUPPORTED
static const struct SpriteFunc VULKAN_SPRITE = {sprite_vulkan_init, sprite_vulkan_delete, sprite_vulkan_render, sprite_vulkan_update_uniforms};
#endif
#ifdef DIRECTX_12_API_SUPPORTED
static const struct SpriteFunc DIRECTX_12_SPRITE = {sprite_directx_12_init, sprite_directx_12_delete, sprite_directx_12_render, sprite_directx_12_update_uniforms};
#endif

struct Sprite {
  struct SpriteFunc sprite_func;
  struct SpriteCommon sprite_common;
};

uint_fast8_t sprite_init(struct Sprite *sprite, struct APICommon *api_common, struct Shader *shader, struct Texture *texture, size_t num);
void sprite_delete(struct Sprite *sprite, struct APICommon *api_common);
void sprite_render(struct Sprite *sprite, struct GBufferCommon *gbuffer_common);
void sprite_update_uniforms(struct Sprite *sprite, struct APICommon *api_common, struct GBufferCommon *gbuffer_common);
void sprite_recreate(struct Sprite *sprite, struct APICommon *api_common, size_t num);
