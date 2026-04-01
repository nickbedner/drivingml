#pragma once

#include "mana/core/corecommon.h"
#include "mana/core/graphics/apis/api.h"
#include "mana/core/graphics/render/gbuffer/gbuffer.h"
#include "mana/core/graphics/render/postprocess/postprocess.h"
#include "mana/core/graphics/render/surface.h"
#include "mana/core/graphics/render/swapchain/swapchain.h"

struct RendererSettings {
  u32 width;
  u32 height;
  enum API_TYPE preferred_api_type;

  u8 msaa_samples;
  u8 supersample_scale;
  b8 vsync;
};
