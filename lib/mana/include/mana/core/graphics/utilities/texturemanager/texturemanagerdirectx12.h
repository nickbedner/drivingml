#pragma once

#include "mana/core/graphics/utilities/texture/texture.h"
#include "mana/core/graphics/utilities/texturemanager/texturemanagercommon.h"

u8 texture_manager_directx_12_init(struct TextureManagerCommon* texture_manager, struct APICommon* api_common);
void texture_manager_directx_12_delete(struct TextureManagerCommon* texture_manager, struct APICommon* api_common);
u8 texture_manager_directx_12_add(struct TextureManagerCommon* texture_manager, struct APICommon* api_common, struct TextureSettings texture_settings);
