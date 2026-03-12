#pragma once

#include "mana/graphics/render/gbuffer/gbuffercommon.h"
#include "mana/graphics/render/swapchain/swapchain.h"
#include "mana/graphics/shaders/shader.h"
#include "mana/graphics/utilities/mesh/mesh.h"

#define WATER_SHADER_COLOR_ATTACHEMENTS 2
#define WATER_SHADER_VERTEX_ATTRIBUTES 2

struct WaterShader {
  struct Shader shader;
  void* extra_data;
};

uint_fast8_t water_shader_init(struct WaterShader* sprite_shader, struct APICommon* api_common, uint32_t width, uint32_t height, uint_fast8_t supersample_scale, struct GBufferCommon* gbuffer_common, const uint_fast32_t msaa_samples, uint_fast32_t descriptors);
void water_shader_delete(struct WaterShader* sprite_shader, struct APICommon* api_common);
