#pragma once

#include "mana/core/graphics/apis/api.h"
#ifdef VULKAN_API_SUPPORTED
#include "mana/core/graphics/utilities/spritemanager/spritemanagervulkan.h"
#endif
#ifdef DIRECTX_12_API_SUPPORTED
#include "mana/core/graphics/utilities/spritemanager/spritemanagerdirectx12.h"
#endif

struct SpriteManagerFunc {
  u8 (*sprite_manager_init_sprite_pool)(struct SpriteManagerCommon*, struct APICommon*, u32, u32, u8, struct GBufferCommon*, u8, uint_fast32_t);
  void (*sprite_manager_delete_sprite)(struct SpriteManagerCommon*, struct APICommon*);
  void (*sprite_manager_add_sprite)(struct SpriteManagerCommon*, struct APICommon*, struct Sprite*, size_t);
};

#ifdef VULKAN_API_SUPPORTED
global const struct SpriteManagerFunc VULKAN_SPRITE_MANAGER = {sprite_manager_vulkan_init_sprite_pool, sprite_manager_vulkan_delete_sprite, sprite_manager_vulkan_add_sprite};
#endif
#ifdef DIRECTX_12_API_SUPPORTED
global const struct SpriteManagerFunc DIRECTX_12_SPRITE_MANAGER = {sprite_manager_directx_12_init_sprite_pool, sprite_manager_directx_12_delete_sprite, sprite_manager_directx_12_add_sprite};
#endif

struct SpriteManager {
  struct SpriteManagerFunc sprite_manager_func;
  struct SpriteManagerCommon sprite_manager_common;
};

void sprite_manager_init(struct SpriteManager* sprite_manager, struct TextureManager* texture_manager, struct APICommon* api_common, u32 width, u32 height, u8 supersample_scale, struct GBufferCommon* gbuffer_common, u8 msaa_samples, uint_fast32_t descriptors);
void sprite_manager_delete(struct SpriteManager* sprite_manager, struct APICommon* api_common);
void sprite_manager_resize(struct SpriteManager* sprite_manager, struct APICommon* api_common, uint_fast32_t width, uint_fast32_t height, u8 supersample_scale);
struct Sprite* sprite_manager_add_sprite(struct SpriteManager* sprite_manager, struct APICommon* api_common, const char* texture_name);
void sprite_manager_remove(struct SpriteManager* sprite_manager, struct APICommon* api_common, size_t sprite_num);
void sprite_manager_update_uniforms(struct SpriteManager* sprite_manager, struct APICommon* api_common, struct GBufferCommon* gbuffer_common);
void sprite_manager_render(struct SpriteManager* sprite_manager, struct GBufferCommon* gbuffer_common, vec4d sort_key);
void sprite_manager_update(struct SpriteManager* sprite_manager, r64 delta_time);

struct Sprite* sprite_manager_sprite_get_handle(struct SpriteManager* sprite_manager, size_t num);
