#pragma once

#include "mana/core/graphics/apis/api.h"
#include "mana/core/graphics/graphicscommon.h"
#include "mana/core/graphics/render/postprocess/postprocesscommon.h"
#include "mana/core/graphics/render/swapchain/swapchaincommon.h"
#include "mana/core/graphics/shaders/blitshader.h"
#include "mana/core/graphics/shaders/shader.h"
#include "mana/core/graphics/utilities/mesh/mesh.h"

u8 swap_chain_directx_12_init(struct SwapChainCommon* swap_chain_common, struct APICommon* api_common, uint_fast32_t width, uint_fast32_t height, b8 vsync, void* extra_data);
void swap_chain_directx_12_delete(struct SwapChainCommon* swap_chain_common, struct APICommon* api_common);
u8 swap_chain_directx_12_resize(struct SwapChainCommon* swap_chain_common, struct APICommon* api_common);
void swap_chain_directx_12_prepare_delete(struct SwapChainCommon* swap_chain_common, struct APICommon* api_common);
u8 swap_chain_directx_12_blit_init(struct SwapChainCommon* swap_chain_common, struct APICommon* api_common, struct PostProcessCommon* post_process_common);
u8 swap_chain_directx_12_blit_update(struct SwapChainCommon* swap_chain_common, struct APICommon* api_common, struct PostProcessCommon* post_process_common);
u8 swap_chain_directx_12_blit_render(struct SwapChainCommon* swap_chain_common, struct PostProcessCommon* post_process_common, u8 swapchain_num);
b8 swap_chain_directx_12_wait_for_fences(struct SwapChainCommon* swap_chain_common, struct APICommon* api_common, size_t frame);
u8 swap_chain_directx_12_end_frame(struct SwapChainCommon* swap_chain_common, struct PostProcessCommon* post_process_common, struct APICommon* api_common);
