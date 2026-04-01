#pragma once

#include "mana/core/graphics/apis/api.h"
#ifdef VULKAN_API_SUPPORTED
#include "mana/core/graphics/render/postprocess/postprocessvulkan.h"
#endif
#ifdef DIRECTX_12_API_SUPPORTED
#include "mana/core/graphics/render/postprocess/postprocessdirectx12.h"
#endif

struct PostProcessFunc {
  u8 (*post_process_init)(struct PostProcessCommon*, struct APICommon*, struct SwapChainCommon*);
  void (*post_process_delete)(struct PostProcessCommon*, struct APICommon*);
  u8 (*post_process_resize)(struct PostProcessCommon*, struct APICommon*, struct SwapChainCommon*);
  u8 (*post_process_resolve_init)(struct PostProcessCommon*, struct APICommon*, struct GBuffer*, struct SwapChainCommon*);
  u8 (*post_process_resolve_update)(struct PostProcessCommon*, struct APICommon*, struct GBuffer*, struct SwapChainCommon*);
  u8 (*post_process_resolve_render)(struct PostProcessCommon*, struct APICommon*, struct GBuffer*, struct SwapChainCommon*);
};

#ifdef VULKAN_API_SUPPORTED
global const struct PostProcessFunc VULKAN_POST_PROCESS = {post_process_vulkan_init, post_process_vulkan_delete, post_process_vulkan_resize, post_process_vulkan_resolve_init, post_process_vulkan_resolve_update, post_process_vulkan_resolve_render};
#endif
#ifdef DIRECTX_12_API_SUPPORTED
global const struct PostProcessFunc DIRECTX_12_POST_PROCESS = {post_process_directx_12_init, post_process_directx_12_delete, post_process_directx_12_resize, post_process_directx_12_resolve_init, post_process_directx_12_resolve_update, post_process_directx_12_resolve_render};
#endif

struct PostProcess {
  struct PostProcessFunc post_process_func;
  struct PostProcessCommon post_process_common;
};

u8 post_process_init(struct PostProcess* post_process, struct APICommon* api_common, struct SwapChainCommon* swap_chain_common);
u8 post_process_delete(struct PostProcess* post_process, struct APICommon* api_common);
u8 post_process_resize(struct PostProcess* post_process, struct APICommon* api_common, struct SwapChainCommon* swap_chain_common);
u8 post_process_start(struct PostProcess* post_process, struct SwapChainCommon* swap_chain_common);
u8 post_process_stop(struct PostProcess* post_process, struct APICommon* api_common);

u8 post_process_resolve_init(struct PostProcess* post_process, struct APICommon* api_common, struct GBuffer* gbuffer, struct SwapChainCommon* swap_chain_common);
u8 post_process_resolve_update(struct PostProcess* post_process, struct APICommon* api_common, struct GBuffer* gbuffer, struct SwapChainCommon* swap_chain_common);
u8 post_process_resolve_render(struct PostProcess* post_process, struct APICommon* api_common, struct GBuffer* gbuffer, struct SwapChainCommon* swap_chain_common);
