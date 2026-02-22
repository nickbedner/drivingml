#pragma once

#include "mana/graphics/render/gbuffer/gbuffercommon.h"

uint_fast8_t gbuffer_directx_12_init(struct GBufferCommon *gbuffer_common, struct APICommon *api_common, struct SwapChainCommon *swap_chain_common, const uint_fast32_t msaa_samples);
void gbuffer_directx_12_delete(struct GBufferCommon *gbuffer_common, struct APICommon *api_common, const uint_fast32_t msaa_samples);
uint_fast8_t gbuffer_directx_12_resize(struct GBufferCommon *gbuffer_common, struct APICommon *api_common, struct SwapChainCommon *swap_chain_common, const uint_fast32_t msaa_samples);
uint_fast8_t gbuffer_directx_12_start(struct GBufferCommon *gbuffer_common, struct SwapChainCommon *swap_chain_common, const uint_fast32_t msaa_samples);
uint_fast8_t gbuffer_directx_12_stop(struct GBufferCommon *gbuffer_common, struct APICommon *api_common, const uint_fast32_t msaa_samples);
