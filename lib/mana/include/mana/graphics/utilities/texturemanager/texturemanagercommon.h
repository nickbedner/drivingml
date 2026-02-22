#pragma once

#include <stdarg.h>

#include "mana/graphics/apis/api.h"
#include "mana/storage/storage.h"

#define TEXTURE_MANAGER_HEAP_SIZE_MAX 2048

#ifdef VULKAN_API_SUPPORTED
struct TextureManagerVulkan {
  uint_fast8_t placeholder;
};
#endif

#ifdef DIRECTX_12_API_SUPPORTED
struct TextureManagerDirectX12 {
  ID3D12DescriptorHeap *srv_heap;               // the descriptor heap containing the SRV for the texture
  D3D12_CPU_DESCRIPTOR_HANDLE cpu_heap_handle;  // handle to the CPU-side descriptor heap
  D3D12_GPU_DESCRIPTOR_HANDLE gpu_heap_handle;  // handle to the GPU-side descriptor heap
  UINT descriptor_size;
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
