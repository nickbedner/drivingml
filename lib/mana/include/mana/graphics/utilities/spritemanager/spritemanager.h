#pragma once

#include "mana/graphics/apis/api.h"
#ifdef VULKAN_API_SUPPORTED
#include "mana/graphics/utilities/spritemanager/spritemanagervulkan.h"
#endif
#ifdef DIRECTX_12_API_SUPPORTED
#include "mana/graphics/utilities/spritemanager/spritemanagerdirectx12.h"
#endif

struct SpriteManagerFunc {
  uint_fast8_t (*sprite_manager_init_sprite_pool)(struct SpriteManagerCommon*, struct APICommon*, uint32_t, uint32_t, uint_fast8_t, struct GBufferCommon*, uint_fast8_t, uint_fast32_t);
  void (*sprite_manager_delete_sprite)(struct SpriteManagerCommon*, struct APICommon*);
  void (*sprite_manager_add_sprite)(struct SpriteManagerCommon*, struct APICommon*, struct Sprite*, size_t);
};

#ifdef VULKAN_API_SUPPORTED
static const struct SpriteManagerFunc VULKAN_SPRITE_MANAGER = {sprite_manager_vulkan_init_sprite_pool, sprite_manager_vulkan_delete_sprite, sprite_manager_vulkan_add_sprite};
#endif
#ifdef DIRECTX_12_API_SUPPORTED
static const struct SpriteManagerFunc DIRECTX_12_SPRITE_MANAGER = {sprite_manager_directx_12_init_sprite_pool, sprite_manager_directx_12_delete_sprite, sprite_manager_directx_12_add_sprite};
#endif

struct SpriteManager {
  struct SpriteManagerFunc sprite_manager_func;
  struct SpriteManagerCommon sprite_manager_common;
};

void sprite_manager_init(struct SpriteManager* sprite_manager, struct TextureManager* texture_manager, struct APICommon* api_common, uint32_t width, uint32_t height, uint_fast8_t supersample_scale, struct GBufferCommon* gbuffer_common, uint_fast8_t msaa_samples, uint_fast32_t descriptors);
void sprite_manager_delete(struct SpriteManager* sprite_manager, struct APICommon* api_common);
void sprite_manager_resize(struct SpriteManager* sprite_manager, struct APICommon* api_common, uint_fast32_t width, uint_fast32_t height, uint_fast8_t supersample_scale);
struct Sprite* sprite_manager_add_sprite(struct SpriteManager* sprite_manager, struct APICommon* api_common, wchar_t* texture_name);
void sprite_manager_remove(struct SpriteManager* sprite_manager, struct APICommon* api_common, size_t sprite_num);
void sprite_manager_update_uniforms(struct SpriteManager* sprite_manager, struct APICommon* api_common, struct GBufferCommon* gbuffer_common);
void sprite_manager_render(struct SpriteManager* sprite_manager, struct GBufferCommon* gbuffer_common, vec4d sort_key);
void sprite_manager_update(struct SpriteManager* sprite_manager, double delta_time);

struct Sprite* sprite_manager_sprite_get_handle(struct SpriteManager* sprite_manager, size_t num);
