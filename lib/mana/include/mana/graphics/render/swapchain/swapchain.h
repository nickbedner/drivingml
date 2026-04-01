#pragma once

#include "mana/graphics/apis/api.h"

#ifdef VULKAN_API_SUPPORTED
#include "mana/graphics/render/swapchain/swapchainvulkan.h"
#endif
#ifdef DIRECTX_12_API_SUPPORTED
#include "mana/graphics/render/swapchain/swapchaindirectx12.h"
#endif

struct APICommon;
struct SwapChain;

struct SwapChainFunc {
  u8 (*swap_chain_init)(struct SwapChainCommon*, struct APICommon*, uint_fast32_t, uint_fast32_t, b8, void*);
  void (*swap_chain_delete)(struct SwapChainCommon*, struct APICommon*);
  u8 (*swap_chain_resize)(struct SwapChainCommon*, struct APICommon*);
  void (*swap_chain_prepare_delete)(struct SwapChainCommon*, struct APICommon*);
  u8 (*swap_chain_blit_init)(struct SwapChainCommon*, struct APICommon*, struct PostProcessCommon*);
  u8 (*swap_chain_blit_update)(struct SwapChainCommon*, struct APICommon*, struct PostProcessCommon*);
  u8 (*swap_chain_blit_render)(struct SwapChainCommon*, struct PostProcessCommon*, u8);
  b8 (*swap_chain_wait_for_fences)(struct SwapChainCommon*, struct APICommon*, size_t);
  u8 (*swap_chain_end_frame)(struct SwapChainCommon*, struct PostProcessCommon*, struct APICommon*);
};

#ifdef VULKAN_API_SUPPORTED
global const struct SwapChainFunc VULKAN_SWAP_CHAIN = {swap_chain_vulkan_init, swap_chain_vulkan_delete, swap_chain_vulkan_resize, swap_chain_vulkan_prepare_delete, swap_chain_vulkan_blit_init, swap_chain_vulkan_blit_update, swap_chain_vulkan_blit_render, swap_chain_vulkan_wait_for_fences, swap_chain_vulkan_end_frame};
#endif
#ifdef DIRECTX_12_API_SUPPORTED
global const struct SwapChainFunc DIRECTX_12_SWAP_CHAIN = {swap_chain_directx_12_init, swap_chain_directx_12_delete, swap_chain_directx_12_resize, swap_chain_directx_12_prepare_delete, swap_chain_directx_12_blit_init, swap_chain_directx_12_blit_update, swap_chain_directx_12_blit_render, swap_chain_directx_12_wait_for_fences, swap_chain_directx_12_end_frame};
#endif

struct SwapChain {
  struct SwapChainFunc swap_chain_func;
  struct SwapChainCommon swap_chain_common;
};

u8 swap_chain_init(struct SwapChain* swap_chain, struct APICommon* api_common, uint_fast32_t width, uint_fast32_t height, u8 supersample_scale, b8 vsync, void* extra_data);
void swap_chain_delete(struct SwapChain* swap_chain, struct APICommon* api_common);
u8 swap_chain_resize(struct SwapChain* swap_chain, struct APICommon* api_common, uint_fast32_t width, uint_fast32_t height, u8 supersample_scale);
void swap_chain_prepare_delete(struct SwapChain* swap_chain, struct APICommon* api_common);

u8 swap_chain_blit_init(struct SwapChain* swap_chain, struct APICommon* api_common, struct PostProcessCommon* post_process_common);
u8 swap_chain_blit_update(struct SwapChain* swap_chain, struct APICommon* api_common, struct PostProcessCommon* post_process_common);
void swap_chain_blit_delete(struct SwapChain* swap_chain, struct APICommon* api_common);
u8 swap_chain_blit_render(struct SwapChain* swap_chain, struct PostProcessCommon* post_process_common);
b8 swap_chain_wait_for_fences(struct SwapChain* swap_chain, struct APICommon* api_common, size_t frame);
u8 swap_chain_end_frame(struct SwapChain* swap_chain, struct PostProcessCommon* post_process_common, struct APICommon* api_common);
