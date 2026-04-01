#pragma once

#include <mana/graphics/utilities/texturemanager/texturemanager.h>

#include "mana/graphics/entities/sprite/sprite.h"
#include "mana/graphics/utilities/spritemanager/spritemanagercommon.h"

u8 sprite_manager_directx_12_init_sprite_pool(struct SpriteManagerCommon* sprite_manager_common, struct APICommon* api_common, u32 width, u32 height, u8 supersample_scale, struct GBufferCommon* gbuffer_common, u8 msaa_samples, uint_fast32_t descriptors);
void sprite_manager_directx_12_delete_sprite(struct SpriteManagerCommon* sprite_manager_common, struct APICommon* api_common);
void sprite_manager_directx_12_add_sprite(struct SpriteManagerCommon* sprite_manager_common, struct APICommon* api_common, struct Sprite* sprite, size_t sprite_num);
