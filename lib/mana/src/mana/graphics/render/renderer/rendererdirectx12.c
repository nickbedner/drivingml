#include "mana/graphics/render/renderer/rendererdirectx12.h"

uint8_t directx_12_renderer_init(struct APICommon *api_common, struct Surface *surface, struct SwapChain *swap_chain, struct GBuffer *gbuffer, struct PostProcess *post_process, struct RendererSettings *renderer_settings) {
  return 0;
}

void directx_12_renderer_delete(struct APICommon *api_common, struct Surface *surface, struct SwapChain *swap_chain, struct GBuffer *gbuffer, struct PostProcess *post_process, struct RendererSettings *renderer_settings) {
}

void directx_12_renderer_wait_for_device(struct APICommon *api_common) {
  // Increment the fence value.
  api_common->directx_12_api.fence_value++;

  // Signal the fence with the updated value from the GPU side.
  api_common->directx_12_api.command_queue->lpVtbl->Signal(api_common->directx_12_api.command_queue, api_common->directx_12_api.fence, api_common->directx_12_api.fence_value);

  // Wait for the fence to reach the specified value.
  if (api_common->directx_12_api.fence->lpVtbl->GetCompletedValue(api_common->directx_12_api.fence) < api_common->directx_12_api.fence_value) {
    api_common->directx_12_api.fence->lpVtbl->SetEventOnCompletion(api_common->directx_12_api.fence, api_common->directx_12_api.fence_value, api_common->directx_12_api.fence_event);

    // Wait for the event to be signaled, indicating the GPU has finished.
    WaitForSingleObject(api_common->directx_12_api.fence_event, INFINITE);
  }
}
