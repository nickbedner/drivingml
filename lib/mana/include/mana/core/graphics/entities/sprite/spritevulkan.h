#pragma once

#include "mana/core/graphics/entities/sprite/spritecommon.h"

u8 sprite_vulkan_init(struct SpriteCommon* sprite_common, struct APICommon* api_common, struct Shader* shader, struct Texture* texture, size_t sprite_num);
void sprite_vulkan_delete(struct SpriteCommon* sprite_common, struct APICommon* api_common);
void sprite_vulkan_render(struct SpriteCommon* sprite_common, struct GBufferCommon* gbuffer_common);
void sprite_vulkan_update_uniforms(struct SpriteCommon* sprite_common, struct APICommon* api_common, struct GBufferCommon* gbuffer_common);
