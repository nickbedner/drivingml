#pragma once

#include "mana/graphics/entities/sprite/spritecommon.h"

uint_fast8_t sprite_directx_12_init(struct SpriteCommon *sprite_common, struct APICommon *api_common, struct Shader *shader, struct Texture *texture, size_t num);
void sprite_directx_12_delete(struct SpriteCommon *sprite_common, struct APICommon *api_common);
void sprite_directx_12_render(struct SpriteCommon *sprite_common, struct GBufferCommon *gbuffer_common);
void sprite_directx_12_update_uniforms(struct SpriteCommon *sprite_common, struct APICommon *api_common, struct GBufferCommon *gbuffer_common);
