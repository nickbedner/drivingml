#pragma once

#include "mana/graphics/shaders/shadercommon.h"

u8 shader_directx_12_init(struct ShaderCommon* shader_common, struct APICommon* api_common, u32 width, u32 height, u8 supersample_scale);
u8 shader_compute_directx_12_init(struct ShaderCommon* shader, struct APICommon* api_common);
void shader_directx_12_delete(struct ShaderCommon* shader_common, struct APICommon* api_common);
void shader_directx_12_resize(struct ShaderCommon* shader_common, struct APICommon* api_common, u32 width, u32 height, u8 supersample_scale);
