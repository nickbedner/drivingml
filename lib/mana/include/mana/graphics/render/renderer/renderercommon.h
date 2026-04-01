#pragma once

#include "mana/core/corecommon.h"
#include "mana/graphics/apis/api.h"
#include "mana/graphics/render/gbuffer/gbuffer.h"
#include "mana/graphics/render/postprocess/postprocess.h"
#include "mana/graphics/render/surface.h"
#include "mana/graphics/render/swapchain/swapchain.h"

struct RendererSettings {
  u32 width;
  u32 height;
  enum API_TYPE preferred_api_type;

  u8 msaa_samples;
  u8 supersample_scale;
  b8 vsync;
};
