#pragma once

#include "mana/graphics/utilities/texture/pngloader.h"
#include "mana/graphics/utilities/texture/texturecommon.h"

uint8_t texture_vulkan_init(struct TextureCommon* texture_common, struct TextureManagerCommon* texture_manager_common, struct APICommon* api_common, void* pixels);
void texture_vulkan_delete(struct TextureCommon* texture_common, struct APICommon* api_common);
