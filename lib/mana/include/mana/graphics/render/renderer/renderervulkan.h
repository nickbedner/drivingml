#pragma once

#include "mana/graphics/render/renderer/renderercommon.h"

u8 vulkan_renderer_init(struct APICommon* api_common, struct Surface* surface, struct SwapChain* swap_chain, struct GBuffer* gbuffer, struct PostProcess* post_process, struct RendererSettings* renderer_settings);
void vulkan_renderer_delete(struct APICommon* api_common, struct Surface* surface, struct SwapChain* swap_chain, struct GBuffer* gbuffer, struct PostProcess* post_process, struct RendererSettings* renderer_settings);
void vulkan_renderer_wait_for_device(struct APICommon* api_common);
