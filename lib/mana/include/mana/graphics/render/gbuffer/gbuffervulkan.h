#pragma once

#include "mana/graphics/render/gbuffer/gbuffercommon.h"

u8 gbuffer_vulkan_init(struct GBufferCommon* gbuffer_common, struct APICommon* api_common, struct SwapChainCommon* swap_chain_common, const uint_fast32_t msaa_samples);
void gbuffer_vulkan_delete(struct GBufferCommon* gbuffer_common, struct APICommon* api_common, const uint_fast32_t msaa_samples);
u8 gbuffer_vulkan_resize(struct GBufferCommon* gbuffer_common, struct APICommon* api_common, struct SwapChainCommon* swap_chain_common, const uint_fast32_t msaa_samples);
u8 gbuffer_vulkan_start(struct GBufferCommon* gbuffer_common, struct SwapChainCommon* swap_chain_common, const uint_fast32_t msaa_samples);
u8 gbuffer_vulkan_stop(struct GBufferCommon* gbuffer_common, struct APICommon* api_common, const uint_fast32_t msaa_samples);
