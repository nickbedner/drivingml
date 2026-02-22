#pragma once

#include "mana/graphics/apis/api.h"
#include "mana/graphics/graphicscommon.h"
#include "mana/graphics/render/postprocess/postprocesscommon.h"
#include "mana/graphics/render/swapchain/swapchaincommon.h"
#include "mana/graphics/shaders/blitshader.h"
#include "mana/graphics/shaders/shader.h"
#include "mana/graphics/utilities/mesh/mesh.h"

uint_fast8_t swap_chain_directx_12_init(struct SwapChainCommon *swap_chain_common, struct APICommon *api_common, uint_fast32_t width, uint_fast32_t height, void *extra_data);
void swap_chain_directx_12_delete(struct SwapChainCommon *swap_chain_common, struct APICommon *api_common);
uint_fast8_t swap_chain_directx_12_resize(struct SwapChainCommon *swap_chain_common, struct APICommon *api_common);
void swap_chain_directx_12_prepare_delete(struct SwapChainCommon *swap_chain_common, struct APICommon *api_common);
uint_fast8_t swap_chain_directx_12_blit_init(struct SwapChainCommon *swap_chain_common, struct APICommon *api_common, struct PostProcessCommon *post_process_common);
uint_fast8_t swap_chain_directx_12_blit_update(struct SwapChainCommon *swap_chain_common, struct APICommon *api_common, struct PostProcessCommon *post_process_common);
uint_fast8_t swap_chain_directx_12_blit_render(struct SwapChainCommon *swap_chain_common, struct PostProcessCommon *post_process_common, uint_fast8_t swapchain_num);
bool swap_chain_directx_12_wait_for_fences(struct SwapChainCommon *swap_chain_common, struct APICommon *api_common, size_t frame);
uint_fast8_t swap_chain_directx_12_end_frame(struct SwapChainCommon *swap_chain_common, struct PostProcessCommon *post_process_common, struct APICommon *api_common);
