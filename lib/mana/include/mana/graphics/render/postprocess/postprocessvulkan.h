#pragma once

#include "mana/graphics/render/postprocess/postprocesscommon.h"

uint_fast8_t post_process_vulkan_init(struct PostProcessCommon *post_process_common, struct APICommon *api_common, struct SwapChainCommon *swap_chain_common);
void post_process_vulkan_delete(struct PostProcessCommon *post_process_common, struct APICommon *api_common);
uint_fast8_t post_process_vulkan_resize(struct PostProcessCommon *post_process_common, struct APICommon *api_common, struct SwapChainCommon *swap_chain_common);

uint_fast8_t post_process_vulkan_resolve_init(struct PostProcessCommon *post_process_common, struct APICommon *api_common, struct GBuffer *gbuffer, struct SwapChainCommon *swap_chain_common);
uint_fast8_t post_process_vulkan_resolve_update(struct PostProcessCommon *post_process_common, struct APICommon *api_common, struct GBuffer *gbuffer, struct SwapChainCommon *swap_chain_common);
uint_fast8_t post_process_vulkan_resolve_render(struct PostProcessCommon *post_process_common, struct APICommon *api_common, struct GBuffer *gbuffer, struct SwapChainCommon *swap_chain_common);
