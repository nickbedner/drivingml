#pragma once

#include <mana/core/graphics/shaders/spriteshader.h>
#include <stdarg.h>

#include "mana/core/graphics/apis/api.h"
#include "mana/core/storage/storage.h"

#ifdef VULKAN_API_SUPPORTED
struct SpriteManagerVulkan {
  VkDescriptorSet* sprite_descriptor_set;
  VkDescriptorSet* sprite_animation_descriptor_set;
};
#endif

#ifdef DIRECTX_12_API_SUPPORTED
struct SpriteManagerDirectX12 {
  ID3D12DescriptorHeap* sprite_srv_heap;
  ID3D12DescriptorHeap* sprite_animation_srv_heap;
  D3D12_CPU_DESCRIPTOR_HANDLE sprite_cpu_heap_handle;
  D3D12_CPU_DESCRIPTOR_HANDLE sprite_animation_cpu_heap_handle;
  D3D12_GPU_DESCRIPTOR_HANDLE sprite_gpu_heap_handle;
  D3D12_GPU_DESCRIPTOR_HANDLE sprite_animation_gpu_heap_handle;
  UINT sprite_descriptor_size;
  UINT sprite_animation_descriptor_size;
};
#endif

struct SpriteManagerCommon {
  struct SpriteShader sprite_shader;
  struct TextureManager* texture_manager;

  struct ArrayList sprites;
  struct ArrayList sprites_animations;

  union {
#ifdef VULKAN_API_SUPPORTED
    struct SpriteManagerVulkan sprite_manager_vulkan;
#endif
#ifdef DIRECTX_12_API_SUPPORTED
    struct SpriteManagerDirectX12 sprite_manager_directx12;
#endif
  };
};
