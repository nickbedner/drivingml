#pragma once

#include "mana/graphics/shaders/shadercommon.h"

uint_fast8_t shader_directx_12_init(struct ShaderCommon *shader_common, struct APICommon *api_common, uint32_t width, uint32_t height, uint_fast8_t supersample_scale);
uint_fast8_t shader_compute_directx_12_init(struct ShaderCommon *shader, struct APICommon *api_common);
void shader_directx_12_delete(struct ShaderCommon *shader_common, struct APICommon *api_common);
void shader_directx_12_resize(struct ShaderCommon *shader_common, struct APICommon *api_common, uint32_t width, uint32_t height, uint_fast8_t supersample_scale);
