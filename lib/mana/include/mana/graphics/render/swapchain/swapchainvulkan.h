#pragma once

#include "mana/graphics/apis/api.h"
#include "mana/graphics/graphicscommon.h"
#include "mana/graphics/render/postprocess/postprocesscommon.h"
#include "mana/graphics/render/swapchain/swapchaincommon.h"
#include "mana/graphics/shaders/blitshader.h"
#include "mana/graphics/shaders/shader.h"
#include "mana/graphics/utilities/mesh/mesh.h"

struct SwapChainSupportDetails {
  struct Vector formats;
  struct Vector present_modes;
  struct VkSurfaceCapabilitiesKHR capabilities;
};

u8 swap_chain_vulkan_init(struct SwapChainCommon* swap_chain_common, struct APICommon* api_common, uint_fast32_t width, uint_fast32_t height, b8 vsync, void* extra_data);
void swap_chain_vulkan_delete(struct SwapChainCommon* swap_chain_common, struct APICommon* api_common);
u8 swap_chain_vulkan_resize(struct SwapChainCommon* swap_chain_common, struct APICommon* api_common);
void swap_chain_vulkan_prepare_delete(struct SwapChainCommon* swap_chain_common, struct APICommon* api_common);
u8 swap_chain_vulkan_blit_init(struct SwapChainCommon* swap_chain_common, struct APICommon* api_common, struct PostProcessCommon* post_process_common);
u8 swap_chain_vulkan_blit_update(struct SwapChainCommon* swap_chain_common, struct APICommon* api_common, struct PostProcessCommon* post_process_common);
u8 swap_chain_vulkan_blit_render(struct SwapChainCommon* swap_chain_common, struct PostProcessCommon* post_process_common, u8 swap_chain_num);
b8 swap_chain_vulkan_wait_for_fences(struct SwapChainCommon* swap_chain_common, struct APICommon* api_common, size_t frame);
u8 swap_chain_vulkan_end_frame(struct SwapChainCommon* swap_chain_common, struct PostProcessCommon* post_process_common, struct APICommon* api_common);
