#pragma once

#include "mana/graphics/shaders/shadercommon.h"

u8 shader_vulkan_init(struct ShaderCommon* shader_common, struct APICommon* api_common, u32 width, u32 height, u8 supersample_scale);
u8 shader_compute_vulkan_init(struct ShaderCommon* shader, struct APICommon* api_common);
void shader_vulkan_delete(struct ShaderCommon* shader_common, struct APICommon* api_common);
void shader_vulkan_resize(struct ShaderCommon* shader_common, struct APICommon* api_common, u32 width, u32 height, u8 supersample_scale);
