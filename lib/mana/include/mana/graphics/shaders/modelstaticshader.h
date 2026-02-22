#pragma once

#include "mana/graphics/shaders/shader.h"

// Effect for blitting static model to gbuffer

#define MODEL_STATIC_SHADER_COLOR_ATTACHEMENTS 2
#define MODEL_STATIC_SHADER_VERTEX_ATTRIBUTES 4

struct ModelStaticShader {
  struct Shader shader;
};

uint_fast8_t model_static_shader_init(struct ModelStaticShader *model_static_shader, struct APICommon *api_common, uint32_t width, uint32_t height, uint_fast8_t supersample_scale, struct GBufferCommon *gbuffer_common, bool depth_test, const uint_fast32_t msaa_samples, uint_fast32_t descriptors);
void model_static_shader_delete(struct ModelStaticShader *model_static_shader, struct APICommon *api_common);
