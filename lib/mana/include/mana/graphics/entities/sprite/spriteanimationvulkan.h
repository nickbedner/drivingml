#pragma once

#include "mana/graphics/entities/sprite/spriteanimationcommon.h"

uint_fast8_t sprite_animation_vulkan_init(struct SpriteAnimationCommon *sprite_common, struct APICommon *api_common, struct Shader *shader, struct Texture *texture);
void sprite_animation_vulkan_delete(struct SpriteAnimationCommon *sprite_common, struct APICommon *api_common);
void sprite_animation_vulkan_render(struct SpriteAnimationCommon *sprite_common, struct GBufferCommon *gbuffer_common);
void sprite_animation_vulkan_update_uniforms(struct SpriteAnimationCommon *sprite_common, struct APICommon *api_common, struct GBufferCommon *gbuffer_common);
