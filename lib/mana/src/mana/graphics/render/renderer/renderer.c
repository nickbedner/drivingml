#include "mana/graphics/render/renderer/renderer.h"

uint8_t renderer_init(struct Renderer *renderer, struct APICommon *api_common, struct Surface *surface, struct SwapChain *swap_chain, struct GBuffer *gbuffer, struct PostProcess *post_process) {
#ifdef VULKAN_API_SUPPORTED
  if (api_common->api_type == API_VULKAN)
    renderer->renderer_func = VULKAN_RENDERER;
#endif
#ifdef DIRECTX_12_API_SUPPORTED
  if (api_common->api_type == API_DIRECTX12)
    renderer->renderer_func = DIRECTX_12_RENDERER;
#endif

  renderer->renderer_func.renderer_init(api_common, surface, swap_chain, gbuffer, post_process, &(renderer->renderer_settings));

  if (swap_chain_init(swap_chain, api_common, renderer->renderer_settings.width, renderer->renderer_settings.height, renderer->renderer_settings.supersample_scale, surface->hwnd))
    goto renderer_swap_chain_error;
  if (post_process_init(post_process, api_common, &(swap_chain->swap_chain_common)))
    goto renderer_post_process_error;
  if (gbuffer_init(gbuffer, api_common, &(swap_chain->swap_chain_common), renderer->renderer_settings.msaa_samples))
    goto renderer_gbuffer_error;

  if (swap_chain_blit_init(swap_chain, api_common, &(post_process->post_process_common)))
    goto renderer_gbuffer_error;
  if (post_process_resolve_init(post_process, api_common, gbuffer, &(swap_chain->swap_chain_common)))
    goto renderer_gbuffer_error;

  return 0;

renderer_gbuffer_error:
  // gbuffer_delete(gbuffer, api_common, renderer->renderer_settings.msaa_samples);
renderer_post_process_error:
  post_process_delete(post_process, api_common);
renderer_swap_chain_error:
  swap_chain_delete(swap_chain, api_common);

  return 1;
}

void renderer_delete(struct Renderer *renderer, struct APICommon *api_common, struct Surface *surface, struct SwapChain *swap_chain, struct GBuffer *gbuffer, struct PostProcess *post_process) {
  gbuffer_delete(gbuffer, api_common, renderer->renderer_settings.msaa_samples);
  post_process_delete(post_process, api_common);
  swap_chain_delete(swap_chain, api_common);
  renderer->renderer_func.renderer_delete(api_common, surface, swap_chain, gbuffer, post_process, &(renderer->renderer_settings));
}

void renderer_wait_for_device(struct Renderer *renderer, struct APICommon *api_common) {
  renderer->renderer_func.renderer_wait_for_device(api_common);
}
