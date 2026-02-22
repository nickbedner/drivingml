#pragma once

#include <stdbool.h>

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
  uint_fast8_t (*swap_chain_init)(struct SwapChainCommon *, struct APICommon *, uint_fast32_t, uint_fast32_t, void *);
  void (*swap_chain_delete)(struct SwapChainCommon *, struct APICommon *);
  uint_fast8_t (*swap_chain_resize)(struct SwapChainCommon *, struct APICommon *);
  void (*swap_chain_prepare_delete)(struct SwapChainCommon *, struct APICommon *);
  uint_fast8_t (*swap_chain_blit_init)(struct SwapChainCommon *, struct APICommon *, struct PostProcessCommon *);
  uint_fast8_t (*swap_chain_blit_update)(struct SwapChainCommon *, struct APICommon *, struct PostProcessCommon *);
  uint_fast8_t (*swap_chain_blit_render)(struct SwapChainCommon *, struct PostProcessCommon *, uint_fast8_t);
  bool (*swap_chain_wait_for_fences)(struct SwapChainCommon *, struct APICommon *, size_t);
  uint_fast8_t (*swap_chain_end_frame)(struct SwapChainCommon *, struct PostProcessCommon *, struct APICommon *);
};

#ifdef VULKAN_API_SUPPORTED
static const struct SwapChainFunc VULKAN_SWAP_CHAIN = {swap_chain_vulkan_init, swap_chain_vulkan_delete, swap_chain_vulkan_resize, swap_chain_vulkan_prepare_delete, swap_chain_vulkan_blit_init, swap_chain_vulkan_blit_update, swap_chain_vulkan_blit_render, swap_chain_vulkan_wait_for_fences, swap_chain_vulkan_end_frame};
#endif
#ifdef DIRECTX_12_API_SUPPORTED
static const struct SwapChainFunc DIRECTX_12_SWAP_CHAIN = {swap_chain_directx_12_init, swap_chain_directx_12_delete, swap_chain_directx_12_resize, swap_chain_directx_12_prepare_delete, swap_chain_directx_12_blit_init, swap_chain_directx_12_blit_update, swap_chain_directx_12_blit_render, swap_chain_directx_12_wait_for_fences, swap_chain_directx_12_end_frame};
#endif

struct SwapChain {
  struct SwapChainFunc swap_chain_func;
  struct SwapChainCommon swap_chain_common;
};

uint_fast8_t swap_chain_init(struct SwapChain *swap_chain, struct APICommon *api_common, uint_fast32_t width, uint_fast32_t height, uint_fast8_t supersample_scale, void *extra_data);
void swap_chain_delete(struct SwapChain *swap_chain, struct APICommon *api_common);
uint_fast8_t swap_chain_resize(struct SwapChain *swap_chain, struct APICommon *api_common, uint_fast32_t width, uint_fast32_t height, uint_fast8_t supersample_scale);
void swap_chain_prepare_delete(struct SwapChain *swap_chain, struct APICommon *api_common);

uint_fast8_t swap_chain_blit_init(struct SwapChain *swap_chain, struct APICommon *api_common, struct PostProcessCommon *post_process_common);
uint_fast8_t swap_chain_blit_update(struct SwapChain *swap_chain, struct APICommon *api_common, struct PostProcessCommon *post_process_common);
void swap_chain_blit_delete(struct SwapChain *swap_chain, struct APICommon *api_common);
uint_fast8_t swap_chain_blit_render(struct SwapChain *swap_chain, struct PostProcessCommon *post_process_common);
bool swap_chain_wait_for_fences(struct SwapChain *swap_chain, struct APICommon *api_common, size_t frame);
uint_fast8_t swap_chain_end_frame(struct SwapChain *swap_chain, struct PostProcessCommon *post_process_common, struct APICommon *api_common);
