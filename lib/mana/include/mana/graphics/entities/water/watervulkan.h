#pragma once

#include "mana/graphics/entities/water/watercommon.h"

uint_fast8_t water_vulkan_init(struct WaterCommon* water_common, struct APICommon* api_common, struct Shader* shader, struct Texture* texture);
void water_vulkan_delete(struct WaterCommon* water_common, struct APICommon* api_common);
void water_vulkan_render(struct WaterCommon* water_common, struct GBufferCommon* gbuffer_common);
void water_vulkan_update_uniforms(struct WaterCommon* water_common, struct APICommon* api_common, struct GBufferCommon* gbuffer_common, uint32_t width, uint32_t height);
