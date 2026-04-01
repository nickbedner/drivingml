#pragma once

#include "mana/graphics/render/postprocess/postprocesscommon.h"

u8 post_process_directx_12_init(struct PostProcessCommon* post_process_common, struct APICommon* api_common, struct SwapChainCommon* swap_chain_common);
void post_process_directx_12_delete(struct PostProcessCommon* post_process_common, struct APICommon* api_common);
u8 post_process_directx_12_resize(struct PostProcessCommon* post_process_common, struct APICommon* api_common, struct SwapChainCommon* swap_chain_common);

u8 post_process_directx_12_resolve_init(struct PostProcessCommon* post_process_common, struct APICommon* api_common, struct GBuffer* gbuffer, struct SwapChainCommon* swap_chain_common);
u8 post_process_directx_12_resolve_update(struct PostProcessCommon* post_process_common, struct APICommon* api_common, struct GBuffer* gbuffer, struct SwapChainCommon* swap_chain_common);
u8 post_process_directx_12_resolve_render(struct PostProcessCommon* post_process_common, struct APICommon* api_common, struct GBuffer* gbuffer, struct SwapChainCommon* swap_chain_common);
