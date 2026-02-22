#pragma once

#include "mana/graphics/apis/api.h"
#include "mana/graphics/render/gbuffer/gbuffer.h"
#include "mana/graphics/render/swapchain/swapchaincommon.h"
#include "mana/graphics/shaders/blitshader.h"
#include "mana/graphics/shaders/resolveshader.h"
#include "mana/graphics/shaders/shader.h"
#include "mana/graphics/utilities/mesh/mesh.h"

#ifdef VULKAN_API_SUPPORTED
struct PostProcessVulkan {
  VkCommandBuffer command_buffer[POST_PROCESS_PING_PONG];
  VkFramebuffer framebuffer[POST_PROCESS_PING_PONG];
  VkSemaphore semaphore[POST_PROCESS_PING_PONG];
  VkRenderPass render_pass;
  VkSampler texture_sampler;

  VkImage color_images[POST_PROCESS_PING_PONG];
  VkDeviceMemory color_image_memories[POST_PROCESS_PING_PONG];
  VkImageView color_image_views[POST_PROCESS_PING_PONG];

  VkBuffer vertex_buffer;
  VkDeviceMemory vertex_buffer_memory;
  VkBuffer index_buffer;
  VkDeviceMemory index_buffer_memory;

  VkBuffer uniform_buffer;
  VkDeviceMemory uniform_buffers_memory;

  // Note: One set for each resolve
  VkDescriptorSet descriptor_set[4];
};
#endif

#ifdef DIRECTX_12_API_SUPPORTED
struct PostProcessDirectX12 {
  ID3D12Resource *color_textures[POST_PROCESS_PING_PONG];

  ID3D12DescriptorHeap *rtv_descriptor_heap[POST_PROCESS_PING_PONG];
  UINT rtv_descriptor_size[POST_PROCESS_PING_PONG];
  D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle[POST_PROCESS_PING_PONG];
  ID3D12CommandAllocator *command_allocator[POST_PROCESS_PING_PONG];
  ID3D12GraphicsCommandList *command_list[POST_PROCESS_PING_PONG];

  ID3D12Resource *constant_buffer;  // Constant Buffer Resource

  ID3D12Fence *fence[POST_PROCESS_PING_PONG];
  HANDLE fence_event[POST_PROCESS_PING_PONG];
  UINT64 fence_value[POST_PROCESS_PING_PONG];
  UINT frame_index[POST_PROCESS_PING_PONG];

  ID3D12Resource *vertex_buffer;
  D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view;
  ID3D12Resource *index_buffer;
  D3D12_INDEX_BUFFER_VIEW index_buffer_view;

  ID3D12DescriptorHeap *srv_heap[POST_PROCESS_PING_PONG];
  D3D12_CPU_DESCRIPTOR_HANDLE srv_cpu_handle[POST_PROCESS_PING_PONG];
  D3D12_GPU_DESCRIPTOR_HANDLE srv_gpu_handle[POST_PROCESS_PING_PONG];
};
#endif

struct PostProcessCommon {
  uint_fast8_t descriptors;
  struct ResolveShader *resolve_shader;
  struct Mesh blit_fullscreen_triangle;
  bool ping_pong;

  union {
#ifdef VULKAN_API_SUPPORTED
    struct PostProcessVulkan post_process_vulkan;
#endif
#ifdef DIRECTX_12_API_SUPPORTED
    struct PostProcessDirectX12 post_process_directx12;
#endif
  };
};
