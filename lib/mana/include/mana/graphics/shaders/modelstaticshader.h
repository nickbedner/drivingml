#pragma once

#include "mana/graphics/shaders/shader.h"

// Effect for blitting static model to gbuffer

#define MODEL_STATIC_SHADER_COLOR_ATTACHEMENTS 2
#define MODEL_STATIC_SHADER_VERTEX_ATTRIBUTES 4

struct ModelStaticShader {
  struct Shader shader;
};

u8 model_static_shader_init(struct ModelStaticShader* model_static_shader, struct APICommon* api_common, u32 width, u32 height, u8 supersample_scale, struct GBufferCommon* gbuffer_common, b8 depth_test, b8 depth_write, const uint_fast32_t msaa_samples, uint_fast32_t descriptors);
void model_static_shader_delete(struct ModelStaticShader* model_static_shader, struct APICommon* api_common);
