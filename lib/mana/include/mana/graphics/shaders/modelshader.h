#pragma once
#ifndef MODEL_SHADER_H
#define MODEL_SHADER_H

#include "mana/graphics/shaders/shader.h"

// Effect for blitting model to gbuffer

#define MODEL_SHADER_COLOR_ATTACHEMENTS 2
#define MODEL_SHADER_VERTEX_ATTRIBUTES 6

struct ModelShader {
  struct Shader shader;
};

uint_fast8_t model_shader_init(struct ModelShader *model_shader, struct APICommon *api_common, uint32_t width, uint32_t height, uint_fast8_t supersample_scale, struct GBufferCommon *gbuffer_common, bool depth_test, const uint_fast32_t msaa_samples, uint_fast32_t descriptors);
void model_shader_delete(struct ModelShader *model_shader, struct APICommon *api_common);

#endif  // MODEL_SHADER_H
