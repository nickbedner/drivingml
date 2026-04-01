#pragma once

#include "mana/core/graphics/entities/water/watercommon.h"

u8 water_directx_12_init(struct WaterCommon* water_common, struct APICommon* api_common, struct Shader* shader, struct Texture* texture);
void water_directx_12_delete(struct WaterCommon* water_common, struct APICommon* api_common);
void water_directx_12_render(struct WaterCommon* water_common, struct GBufferCommon* gbuffer_common);
void water_directx_12_update_uniforms(struct WaterCommon* water_common, struct APICommon* api_common, struct GBufferCommon* gbuffer_common, u32 width, u32 height);
