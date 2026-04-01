#include "mana/graphics/render/gbuffer/gbuffer.h"

// TODO: Implement sample shading
// https://vulkan-tutorial.com/Multisampling
u8 gbuffer_init(struct GBuffer* gbuffer, struct APICommon* api_common, struct SwapChainCommon* swap_chain_common, const uint_fast32_t msaa_samples) {
#ifdef VULKAN_API_SUPPORTED
  if (api_common->api_type == API_VULKAN)
    gbuffer->gbuffer_func = VULKAN_GBUFFER;
#endif
#ifdef DIRECTX_12_API_SUPPORTED
  if (api_common->api_type == API_DIRECTX_12)
    gbuffer->gbuffer_func = DIRECTX_12_GBUFFER;
#endif

  gbuffer->gbuffer_common.descriptors = 1;
  gbuffer->gbuffer_common.color_clear_value = COLOR_SUN_BLUE;
  gbuffer->gbuffer_common.normal_clear_value = COLOR_TRANSPARENT_BLACK;
  gbuffer->gbuffer_common.depth_clear_value = 0.0f;  // Note: I'm using inverse depth, so 0.0f is the far plane

  return gbuffer->gbuffer_func.gbuffer_init(&(gbuffer->gbuffer_common), api_common, swap_chain_common, msaa_samples);
}

void gbuffer_delete(struct GBuffer* gbuffer, struct APICommon* api_common, const uint_fast32_t msaa_samples) {
  gbuffer->gbuffer_func.gbuffer_delete(&(gbuffer->gbuffer_common), api_common, msaa_samples);
}

u8 gbuffer_resize(struct GBuffer* gbuffer, struct APICommon* api_common, struct SwapChainCommon* swap_chain_common, const uint_fast32_t msaa_samples) {
  gbuffer->gbuffer_func.gbuffer_resize(&(gbuffer->gbuffer_common), api_common, swap_chain_common, msaa_samples);

  return 0;
}
u8 gbuffer_start(struct GBuffer* gbuffer, struct SwapChainCommon* swap_chain_common, const uint_fast32_t msaa_samples) {
  gbuffer->gbuffer_func.gbuffer_start(&(gbuffer->gbuffer_common), swap_chain_common, msaa_samples);

  return 0;
}

u8 gbuffer_stop(struct GBuffer* gbuffer, struct APICommon* api_common, const uint_fast32_t msaa_samples) {
  gbuffer->gbuffer_func.gbuffer_stop(&(gbuffer->gbuffer_common), api_common, msaa_samples);

  return 0;
}
