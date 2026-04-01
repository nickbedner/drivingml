#include "mana/graphics/render/postprocess/postprocess.h"

u8 post_process_init(struct PostProcess* post_process, struct APICommon* api_common, struct SwapChainCommon* swap_chain_common) {
#ifdef VULKAN_API_SUPPORTED
  if (api_common->api_type == API_VULKAN)
    post_process->post_process_func = VULKAN_POST_PROCESS;
#endif
#ifdef DIRECTX_12_API_SUPPORTED
  if (api_common->api_type == API_DIRECTX_12)
    post_process->post_process_func = DIRECTX_12_POST_PROCESS;
#endif

  post_process->post_process_common.ping_pong = FALSE;

  post_process->post_process_common.resolve_shader = (struct ResolveShader*)calloc(1, sizeof(struct ResolveShader));

  post_process->post_process_common.descriptors = 1;

  return post_process->post_process_func.post_process_init(&(post_process->post_process_common), api_common, swap_chain_common);
}

u8 post_process_delete(struct PostProcess* post_process, struct APICommon* api_common) {
  // Note: Kinda hacky way to check if not NULL
  if (post_process->post_process_common.resolve_shader != NULL && post_process->post_process_common.resolve_shader->shader[0].shader_common.shader_settings.num_msaa_samples != 0) {
    resolve_shader_delete(post_process->post_process_common.resolve_shader, api_common);
    free(post_process->post_process_common.resolve_shader);
  }

  if (post_process->post_process_common.blit_fullscreen_triangle.mesh_common.indices != NULL)
    mesh_delete(&(post_process->post_process_common.blit_fullscreen_triangle), api_common);

  post_process->post_process_func.post_process_delete(&(post_process->post_process_common), api_common);

  return 0;
}

u8 post_process_resize(struct PostProcess* post_process, struct APICommon* api_common, struct SwapChainCommon* swap_chain_common) {
  if (post_process->post_process_func.post_process_resize(&(post_process->post_process_common), api_common, swap_chain_common) != 0)
    return 1;

  for (u8 shader_num = 0; shader_num < 4; shader_num++)
    shader_resize(&(post_process->post_process_common.resolve_shader->shader[shader_num]), api_common, swap_chain_common->swap_chain_extent.width, swap_chain_common->swap_chain_extent.height, swap_chain_common->supersample_scale);

  return 0;
}

u8 post_process_resolve_init(struct PostProcess* post_process, struct APICommon* api_common, struct GBuffer* gbuffer, struct SwapChainCommon* swap_chain_common) {
  mesh_init(&(post_process->post_process_common.blit_fullscreen_triangle), MESH_TYPE_TRIANGLE, api_common);
  mesh_fullscreen_triangle(&(post_process->post_process_common.blit_fullscreen_triangle));

  resolve_shader_init(post_process->post_process_common.resolve_shader, api_common, swap_chain_common->swap_chain_extent.width, swap_chain_common->swap_chain_extent.height, swap_chain_common->supersample_scale, 3, post_process->post_process_common.descriptors, MESH_TYPE_TRIANGLE);

  return post_process->post_process_func.post_process_resolve_init(&(post_process->post_process_common), api_common, gbuffer, swap_chain_common);
}

u8 post_process_resolve_update(struct PostProcess* post_process, struct APICommon* api_common, struct GBuffer* gbuffer, struct SwapChainCommon* swap_chain_common) {
  return post_process->post_process_func.post_process_resolve_update(&(post_process->post_process_common), api_common, gbuffer, swap_chain_common);
}

u8 post_process_resolve_render(struct PostProcess* post_process, struct APICommon* api_common, struct GBuffer* gbuffer, struct SwapChainCommon* swap_chain_common) {
  post_process->post_process_func.post_process_resolve_render(&(post_process->post_process_common), api_common, gbuffer, swap_chain_common);

  post_process->post_process_common.ping_pong ^= TRUE;

  return 0;
}
