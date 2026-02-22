#pragma once

#include "mana/graphics/apis/api.h"
#ifdef VULKAN_API_SUPPORTED
#include "mana/graphics/render/gbuffer/gbuffervulkan.h"
#endif
#ifdef DIRECTX_12_API_SUPPORTED
#include "mana/graphics/render/gbuffer/gbufferdirectx12.h"
#endif

struct GBufferFunc {
  uint_fast8_t (*gbuffer_init)(struct GBufferCommon *, struct APICommon *, struct SwapChainCommon *, const uint_fast32_t);
  void (*gbuffer_delete)(struct GBufferCommon *, struct APICommon *, const uint_fast32_t);
  uint_fast8_t (*gbuffer_resize)(struct GBufferCommon *, struct APICommon *, struct SwapChainCommon *, const uint_fast32_t);
  uint_fast8_t (*gbuffer_start)(struct GBufferCommon *, struct SwapChainCommon *, const uint_fast32_t);
  uint_fast8_t (*gbuffer_stop)(struct GBufferCommon *, struct APICommon *, const uint_fast32_t);
};

#ifdef VULKAN_API_SUPPORTED
static const struct GBufferFunc VULKAN_GBUFFER = {gbuffer_vulkan_init, gbuffer_vulkan_delete, gbuffer_vulkan_resize, gbuffer_vulkan_start, gbuffer_vulkan_stop};
#endif
#ifdef DIRECTX_12_API_SUPPORTED
static const struct GBufferFunc DIRECTX_12_GBUFFER = {gbuffer_directx_12_init, gbuffer_directx_12_delete, gbuffer_directx_12_resize, gbuffer_directx_12_start, gbuffer_directx_12_stop};
#endif

struct GBuffer {
  struct GBufferFunc gbuffer_func;
  struct GBufferCommon gbuffer_common;
};

uint_fast8_t gbuffer_init(struct GBuffer *gbuffer, struct APICommon *api_common, struct SwapChainCommon *swap_chain_common, const uint_fast32_t msaa_samples);
void gbuffer_delete(struct GBuffer *gbuffer, struct APICommon *api_common, const uint_fast32_t msaa_samples);
uint_fast8_t gbuffer_resize(struct GBuffer *gbuffer, struct APICommon *api_common, struct SwapChainCommon *swap_chain_common, const uint_fast32_t msaa_samples);
uint_fast8_t gbuffer_start(struct GBuffer *gbuffer, struct SwapChainCommon *swap_chain_common, const uint_fast32_t msaa_samples);
uint_fast8_t gbuffer_stop(struct GBuffer *gbuffer, struct APICommon *api_common, const uint_fast32_t msaa_samples);
