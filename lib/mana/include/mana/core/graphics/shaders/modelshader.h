#pragma once
#ifndef MODEL_SHADER_H
#define MODEL_SHADER_H

#include "mana/core/graphics/shaders/shader.h"

// Effect for blitting model to gbuffer

#define MODEL_SHADER_COLOR_ATTACHEMENTS 2
#define MODEL_SHADER_VERTEX_ATTRIBUTES 6

struct ModelShader {
  struct Shader shader;
};

u8 model_shader_init(struct ModelShader* model_shader, struct APICommon* api_common, u32 width, u32 height, u8 supersample_scale, struct GBufferCommon* gbuffer_common, b8 depth_test, b8 depth_write, const uint_fast32_t msaa_samples, uint_fast32_t descriptors);
void model_shader_delete(struct ModelShader* model_shader, struct APICommon* api_common);

#endif  // MODEL_SHADER_H
