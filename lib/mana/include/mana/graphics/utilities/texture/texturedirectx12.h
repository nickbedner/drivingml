#pragma once

#include "mana/graphics/utilities/texture/texturecommon.h"

uint8_t texture_directx_12_init(struct TextureCommon *texture_common, struct TextureManagerCommon *texture_manager_common, struct APICommon *api_common, struct TextureSettings *texture_settings, void *pixels);
void texture_directx_12_delete(struct TextureCommon *texture_common, struct APICommon *api_common);
