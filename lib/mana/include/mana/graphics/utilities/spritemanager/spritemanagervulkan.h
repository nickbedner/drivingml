#pragma once

#include <mana/graphics/utilities/texturemanager/texturemanager.h>

#include "mana/graphics/entities/sprite/sprite.h"
#include "mana/graphics/entities/sprite/spriteanimation.h"
#include "mana/graphics/utilities/spritemanager/spritemanagercommon.h"


uint_fast8_t sprite_manager_vulkan_init_sprite_pool(struct SpriteManagerCommon *sprite_manager_common, struct APICommon *api_common, uint32_t width, uint32_t height, uint_fast8_t supersample_scale, struct GBufferCommon *gbuffer_common, uint_fast8_t msaa_samples, uint_fast32_t descriptors);
void sprite_manager_vulkan_delete_sprite(struct SpriteManagerCommon *sprite_manager_common, struct APICommon *api_common);
void sprite_manager_vulkan_add_sprite(struct SpriteManagerCommon *sprite_manager_common, struct APICommon *api_common, struct Sprite *sprite, size_t sprite_num);

uint_fast8_t sprite_manager_vulkan_init_sprite_animation_pool(struct SpriteManagerCommon *sprite_manager_common, struct APICommon *api_common, uint32_t width, uint32_t height, uint_fast8_t supersample_scale, struct GBufferCommon *gbuffer_common, uint_fast8_t msaa_samples, uint_fast32_t descriptors);
void sprite_manager_vulkan_delete_sprite_animation(struct SpriteManagerCommon *sprite_manager_common, struct APICommon *api_common);
void sprite_manager_vulkan_add_sprite_animation(struct SpriteManagerCommon *sprite_manager_common, struct APICommon *api_common, struct SpriteAnimation *sprite_animation, size_t sprite_num);
