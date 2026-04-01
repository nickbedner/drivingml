#include "mana/graphics/utilities/spritemanager/spritemanagerdirectx12.h"

u8 sprite_manager_directx_12_init_sprite_pool(struct SpriteManagerCommon* sprite_manager_common, struct APICommon* api_common, u32 width, u32 height, u8 supersample_scale, struct GBufferCommon* gbuffer_common, u8 msaa_samples, uint_fast32_t descriptors) {
  D3D12_DESCRIPTOR_HEAP_DESC heap_desc;
  memset(&heap_desc, 0, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));

  heap_desc.NumDescriptors = descriptors;
  heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

  if (FAILED(api_common->directx_12_api.device->lpVtbl->CreateDescriptorHeap(api_common->directx_12_api.device, &heap_desc, &IID_ID3D12DescriptorHeap, (void**)&(sprite_manager_common->sprite_manager_directx12.sprite_srv_heap))))
    return 1;

  sprite_manager_common->sprite_manager_directx12.sprite_descriptor_size = api_common->directx_12_api.device->lpVtbl->GetDescriptorHandleIncrementSize(api_common->directx_12_api.device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

  sprite_manager_common->sprite_manager_directx12.sprite_srv_heap->lpVtbl->GetCPUDescriptorHandleForHeapStart(sprite_manager_common->sprite_manager_directx12.sprite_srv_heap, &(sprite_manager_common->sprite_manager_directx12.sprite_cpu_heap_handle));
  sprite_manager_common->sprite_manager_directx12.sprite_srv_heap->lpVtbl->GetGPUDescriptorHandleForHeapStart(sprite_manager_common->sprite_manager_directx12.sprite_srv_heap, &(sprite_manager_common->sprite_manager_directx12.sprite_gpu_heap_handle));

  return 0;
}

void sprite_manager_directx_12_delete_sprite(struct SpriteManagerCommon* sprite_manager_common, struct APICommon* api_common) {
}

void sprite_manager_directx_12_add_sprite(struct SpriteManagerCommon* sprite_manager_common, struct APICommon* api_common, struct Sprite* sprite, size_t sprite_num) {
}
