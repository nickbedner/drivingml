#include "mana/graphics/render/swapchain/swapchain.h"

uint_fast8_t swap_chain_init(struct SwapChain* swap_chain, struct APICommon* api_common, uint_fast32_t width, uint_fast32_t height, uint_fast8_t supersample_scale, bool vsync, void* extra_data) {
  swap_chain->swap_chain_common.swap_chain_extent = (struct SwapchainExtent){.width = width, .height = height};
#ifdef _WIN64
  swap_chain->swap_chain_common.hwnd = (HWND)extra_data;
#endif
#ifdef VULKAN_API_SUPPORTED
  if (api_common->api_type == API_VULKAN)
    swap_chain->swap_chain_func = VULKAN_SWAP_CHAIN;
#endif
#ifdef DIRECTX_12_API_SUPPORTED
  if (api_common->api_type == API_DIRECTX_12)
    swap_chain->swap_chain_func = DIRECTX_12_SWAP_CHAIN;
#endif

  // TODO: Load this from options and maybe move later
  swap_chain->swap_chain_common.supersample_scale = supersample_scale;
  swap_chain->swap_chain_common.vsync = vsync;
  swap_chain->swap_chain_common.current_frame = 0;
  swap_chain->swap_chain_common.descriptors = POST_PROCESS_PING_PONG;

  swap_chain->swap_chain_common.blit_shader = (struct BlitShader*)calloc(1, sizeof(struct BlitShader));

  return swap_chain->swap_chain_func.swap_chain_init(&(swap_chain->swap_chain_common), api_common, width, height, vsync, extra_data);
}

void swap_chain_delete(struct SwapChain* swap_chain, struct APICommon* api_common) {
  swap_chain_blit_delete(swap_chain, api_common);

  swap_chain->swap_chain_func.swap_chain_delete(&(swap_chain->swap_chain_common), api_common);
}

uint_fast8_t swap_chain_resize(struct SwapChain* swap_chain, struct APICommon* api_common, uint_fast32_t width, uint_fast32_t height, uint_fast8_t supersample_scale) {
  swap_chain->swap_chain_common.swap_chain_extent = (struct SwapchainExtent){.width = width, .height = height};
  swap_chain->swap_chain_common.supersample_scale = supersample_scale;

  if (swap_chain->swap_chain_func.swap_chain_resize(&(swap_chain->swap_chain_common), api_common) != 0)
    return 1;

  shader_resize(&(swap_chain->swap_chain_common.blit_shader->shader), api_common, width, height, supersample_scale);

  return 0;
}

void swap_chain_prepare_delete(struct SwapChain* swap_chain, struct APICommon* api_common) {
  swap_chain->swap_chain_func.swap_chain_prepare_delete(&(swap_chain->swap_chain_common), api_common);
}

uint_fast8_t swap_chain_blit_init(struct SwapChain* swap_chain, struct APICommon* api_common, struct PostProcessCommon* post_process_common) {
  mesh_init(&(swap_chain->swap_chain_common.blit_fullscreen_triangle), MESH_TYPE_TRIANGLE, api_common);
  mesh_fullscreen_triangle(&(swap_chain->swap_chain_common.blit_fullscreen_triangle));

  blit_shader_init(swap_chain->swap_chain_common.blit_shader, api_common, swap_chain->swap_chain_common.swap_chain_extent.width, swap_chain->swap_chain_common.swap_chain_extent.height, swap_chain->swap_chain_common.supersample_scale, 1, swap_chain->swap_chain_common.descriptors, MESH_TYPE_TRIANGLE);

  return swap_chain->swap_chain_func.swap_chain_blit_init(&(swap_chain->swap_chain_common), api_common, post_process_common);
}

uint_fast8_t swap_chain_blit_update(struct SwapChain* swap_chain, struct APICommon* api_common, struct PostProcessCommon* post_process_common) {
  return swap_chain->swap_chain_func.swap_chain_blit_update(&(swap_chain->swap_chain_common), api_common, post_process_common);
}

void swap_chain_blit_delete(struct SwapChain* swap_chain, struct APICommon* api_common) {
  mesh_delete(&(swap_chain->swap_chain_common.blit_fullscreen_triangle), api_common);

  blit_shader_delete(swap_chain->swap_chain_common.blit_shader, api_common);
  free(swap_chain->swap_chain_common.blit_shader);
}

uint_fast8_t swap_chain_blit_render(struct SwapChain* swap_chain, struct PostProcessCommon* post_process_common) {
  uint_fast8_t error_code = 0;
  for (uint_fast8_t swapchain_num = 0; swapchain_num < MAX_SWAP_CHAIN_FRAMES; swapchain_num++) {
    error_code = swap_chain->swap_chain_func.swap_chain_blit_render(&(swap_chain->swap_chain_common), post_process_common, swapchain_num);

    if (error_code)
      return error_code;
  }

  return 0;
}

bool swap_chain_wait_for_fences(struct SwapChain* swap_chain, struct APICommon* api_common, size_t frame) {
  return swap_chain->swap_chain_func.swap_chain_wait_for_fences(&(swap_chain->swap_chain_common), api_common, frame);
}

uint_fast8_t swap_chain_end_frame(struct SwapChain* swap_chain, struct PostProcessCommon* post_process_common, struct APICommon* api_common) {
  return swap_chain->swap_chain_func.swap_chain_end_frame(&(swap_chain->swap_chain_common), post_process_common, api_common);
}
