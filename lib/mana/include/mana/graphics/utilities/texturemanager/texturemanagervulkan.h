#pragma once

#include "mana/graphics/utilities/texture/texture.h"
#include "mana/graphics/utilities/texturemanager/texturemanagercommon.h"

uint_fast8_t texture_manager_vulkan_init(struct TextureManagerCommon* texture_manager, struct APICommon* api_common);
void texture_manager_vulkan_delete(struct TextureManagerCommon* texture_manager, struct APICommon* api_common);
uint_fast8_t texture_manager_vulkan_add(struct TextureManagerCommon* texture_manager, struct APICommon* api_common, struct TextureSettings texture_settings);
