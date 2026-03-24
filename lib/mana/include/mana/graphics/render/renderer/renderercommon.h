#pragma once

#include "mana/core/corecommon.h"
#include "mana/graphics/apis/api.h"
#include "mana/graphics/render/gbuffer/gbuffer.h"
#include "mana/graphics/render/postprocess/postprocess.h"
#include "mana/graphics/render/surface.h"
#include "mana/graphics/render/swapchain/swapchain.h"

struct RendererSettings {
  uint32_t width;
  uint32_t height;
  enum API_TYPE preferred_api_type;

  uint8_t msaa_samples;
  uint8_t supersample_scale;
  bool vsync;
};
