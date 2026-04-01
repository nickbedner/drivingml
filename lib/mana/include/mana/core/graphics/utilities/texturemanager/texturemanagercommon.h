#pragma once

#include <stdarg.h>

#include "mana/core/graphics/apis/api.h"
#include "mana/core/storage/storage.h"

#define TEXTURE_MANAGER_HEAP_SIZE_MAX 2048

#ifdef VULKAN_API_SUPPORTED
struct TextureManagerVulkan {
  u8 placeholder;
};
#endif

#ifdef DIRECTX_12_API_SUPPORTED
struct TextureManagerDirectX12 {
  ID3D12DescriptorHeap* srv_heap;
  ID3D12DescriptorHeap* sampler_heap;

  D3D12_CPU_DESCRIPTOR_HANDLE srv_cpu_heap_handle;
  D3D12_GPU_DESCRIPTOR_HANDLE srv_gpu_heap_handle;

  D3D12_CPU_DESCRIPTOR_HANDLE sampler_cpu_heap_handle;
  D3D12_GPU_DESCRIPTOR_HANDLE sampler_gpu_heap_handle;

  UINT srv_descriptor_size;
  UINT sampler_descriptor_size;
};
#endif

struct TextureManagerCommon {
  struct Map textures;
  size_t texture_index;

  union {
#ifdef VULKAN_API_SUPPORTED
    struct TextureManagerVulkan texture_manager_vulkan;
#endif
#ifdef DIRECTX_12_API_SUPPORTED
    struct TextureManagerDirectX12 texture_manager_directx12;
#endif
  };
};
