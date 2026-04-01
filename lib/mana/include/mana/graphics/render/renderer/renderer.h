#pragma once

#include "mana/graphics/graphicscommon.h"
#ifdef VULKAN_API_SUPPORTED
#include "mana/graphics/render/renderer/renderervulkan.h"
#endif
#ifdef DIRECTX_12_API_SUPPORTED
#include "mana/graphics/render/renderer/rendererdirectx12.h"
#endif

struct RendererFunc {
  u8 (*renderer_init)(struct APICommon*, struct Surface*, struct SwapChain*, struct GBuffer*, struct PostProcess*, struct RendererSettings*);
  void (*renderer_delete)(struct APICommon*, struct Surface*, struct SwapChain*, struct GBuffer*, struct PostProcess*, struct RendererSettings*);
  void (*renderer_wait_for_device)(struct APICommon*);
};

#ifdef VULKAN_API_SUPPORTED
global const struct RendererFunc VULKAN_RENDERER = {vulkan_renderer_init, vulkan_renderer_delete, vulkan_renderer_wait_for_device};
#endif
#ifdef DIRECTX_12_API_SUPPORTED
global const struct RendererFunc DIRECTX_12_RENDERER = {directx_12_renderer_init, directx_12_renderer_delete, directx_12_renderer_wait_for_device};
#endif
#ifdef METAL_API
global const struct Renderer METAL_RENDERER = {NULL, NULL};
#endif

struct Renderer {
  struct RendererFunc renderer_func;
  struct RendererSettings renderer_settings;
};

u8 renderer_init(struct Renderer* renderer, struct APICommon* api_common, struct Surface* surface, struct SwapChain* swap_chain, struct GBuffer* gbuffer, struct PostProcess* post_process);
void renderer_delete(struct Renderer* renderer, struct APICommon* api_common, struct Surface* surface, struct SwapChain* swap_chain, struct GBuffer* gbuffer, struct PostProcess* post_process);
void renderer_wait_for_device(struct Renderer* renderer, struct APICommon* api_common);
