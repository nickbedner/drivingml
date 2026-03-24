#pragma once

#include "mana/graphics/apis/api.h"
#include "mana/graphics/graphicscommon.h"
#include "mana/graphics/render/swapchain/swapchaincommon.h"

#define GBUFFER_COLOR_ATTACHMENTS 2
#define GBUFFER_TOTAL_DEPENDENCIES 2
#define GBUFFER_TOTAL_ATTACHMENTS 3
#define MULTISAMPLE_GBUFFER_TOTAL_ATTACHMENTS 5

#ifdef VULKAN_API_SUPPORTED
struct GBufferVulkan {
  VkCommandBuffer command_buffer;
  VkFramebuffer framebuffer;
  VkSemaphore semaphore;
  VkRenderPass render_pass;
  VkSampler texture_sampler;

  // Resolve or standard
  VkImage color_image;
  VkDeviceMemory color_image_memory;
  VkImageView color_image_view;
  VkImage normal_image;
  VkDeviceMemory normal_image_memory;
  VkImageView normal_image_view;
  VkImage depth_image;
  VkDeviceMemory depth_image_memory;
  VkImageView depth_image_view;

  // Multisample
  VkImage multisample_color_image;
  VkDeviceMemory multisample_color_image_memory;
  VkImageView multisample_color_image_view;
  VkImage multisample_normal_image;
  VkDeviceMemory multisample_normal_image_memory;
  VkImageView multisample_normal_image_view;
};
#endif

#ifdef DIRECTX_12_API_SUPPORTED
struct GBufferDirectX12 {
  // Resources
  ID3D12Resource* color_texture;
  ID3D12Resource* normal_texture;
  ID3D12Resource* depth_texture;

  ID3D12Resource* multisample_color_texture;
  ID3D12Resource* multisample_normal_texture;

  // Descriptor heaps
  ID3D12DescriptorHeap* rtv_descriptor_heap;
  ID3D12DescriptorHeap* dsv_descriptor_heap;
  ID3D12DescriptorHeap* srv_heap;

  // Command
  ID3D12CommandAllocator* command_allocator;
  ID3D12GraphicsCommandList* command_list;

  // Sync
  ID3D12Fence* fence;
  HANDLE fence_event;
  UINT64 fence_value;

  // Handles
  D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle_color;
  D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle_normal;
  D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle_multisample_color;
  D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle_multisample_normal;

  D3D12_CPU_DESCRIPTOR_HANDLE dsv_handle;

  D3D12_CPU_DESCRIPTOR_HANDLE srv_cpu_handle;
  D3D12_GPU_DESCRIPTOR_HANDLE srv_gpu_handle;

  UINT rtv_descriptor_size;
  UINT dsv_descriptor_size;
  UINT frame_index;

  D3D12_CLEAR_VALUE color_clear_value;
  D3D12_CLEAR_VALUE normal_clear_value;
  D3D12_CLEAR_VALUE depth_clear_value;

  D3D12_VIEWPORT viewport;
  D3D12_RECT scissor_rect;
};
#endif

struct GBufferCommon {
  mat4 projection_matrix;
  mat4 view_matrix;

  vec4 color_clear_value;
  vec4 normal_clear_value;

  float depth_clear_value;
  uint_fast8_t descriptors;
  // Note: Padding for alignment
  uint8_t _pad0[3];

  union {
#ifdef VULKAN_API_SUPPORTED
    struct GBufferVulkan gbuffer_vulkan;
#endif
#ifdef DIRECTX_12_API_SUPPORTED
    struct GBufferDirectX12 gbuffer_directx12;
#endif
  };
};
