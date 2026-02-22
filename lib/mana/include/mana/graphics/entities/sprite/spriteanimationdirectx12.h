#pragma once

#include "mana/graphics/entities/sprite/spriteanimationcommon.h"

uint_fast8_t sprite_animation_directx_12_init(struct SpriteAnimationCommon *sprite_animation_common, struct APICommon *api_common, struct Shader *shader, struct Texture *texture);
void sprite_animation_directx_12_delete(struct SpriteAnimationCommon *sprite_animation_common, struct APICommon *api_common);
void sprite_animation_directx_12_render(struct SpriteAnimationCommon *sprite_animation_common, struct GBufferCommon *gbuffer_common);
void sprite_animation_directx_12_update_uniforms(struct SpriteAnimationCommon *sprite_animation_common, struct APICommon *api_common, struct GBufferCommon *gbuffer_common);
