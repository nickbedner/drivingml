#include "mana/graphics/utilities/texturemanager/texturemanagerdirectx12.h"

uint_fast8_t texture_manager_directx_12_init(struct TextureManagerCommon *texture_manager, struct APICommon *api_common) {
  D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {0};
  heap_desc.NumDescriptors = TEXTURE_MANAGER_HEAP_SIZE_MAX;
  heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

  if (FAILED(api_common->directx_12_api.device->lpVtbl->CreateDescriptorHeap(api_common->directx_12_api.device, &heap_desc, &IID_ID3D12DescriptorHeap, (void **)&(texture_manager->texture_manager_directx12.srv_heap))))
    return 1;

  texture_manager->texture_manager_directx12.descriptor_size = api_common->directx_12_api.device->lpVtbl->GetDescriptorHandleIncrementSize(api_common->directx_12_api.device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

  texture_manager->texture_manager_directx12.srv_heap->lpVtbl->GetCPUDescriptorHandleForHeapStart(texture_manager->texture_manager_directx12.srv_heap, &(texture_manager->texture_manager_directx12.cpu_heap_handle));
  texture_manager->texture_manager_directx12.srv_heap->lpVtbl->GetGPUDescriptorHandleForHeapStart(texture_manager->texture_manager_directx12.srv_heap, &(texture_manager->texture_manager_directx12.gpu_heap_handle));

  return 0;
}

void texture_manager_directx_12_delete(struct TextureManagerCommon *texture_manager, struct APICommon *api_common) {
}

uint_fast8_t texture_manager_directx_12_add(struct TextureManagerCommon *texture_manager, struct APICommon *api_common, struct TextureSettings *texture_settings) {
  return 0;
}
