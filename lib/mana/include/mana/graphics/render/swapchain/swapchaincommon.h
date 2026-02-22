#pragma once

#include "mana/core/corecommon.h"
#include "mana/graphics/apis/api.h"
#include "mana/graphics/graphicscommon.h"
#include "mana/graphics/utilities/mesh/mesh.h"

#define SWAP_CHAIN_UPDATE_FRAMERBUFFER 1

struct SwapchainExtent {
  uint32_t width;
  uint32_t height;
};

#ifdef VULKAN_API_SUPPORTED
struct SwapChainVulkan {
  VkCommandBuffer command_buffer[MAX_SWAP_CHAIN_FRAMES];
  VkFramebuffer framebuffer[MAX_SWAP_CHAIN_FRAMES];
  VkSemaphore semaphore[MAX_SWAP_CHAIN_FRAMES];
  VkRenderPass render_pass;
  VkSampler texture_sampler;

  VkSemaphore image_available_semaphores[MAX_FRAMES_IN_FLIGHT];
  VkSemaphore render_finished_semaphores[MAX_FRAMES_IN_FLIGHT];
  VkFence in_flight_fences[MAX_FRAMES_IN_FLIGHT];
  VkSwapchainKHR swap_chain_khr;
  VkFormat swap_chain_image_format;
  VkSurfaceKHR surface;

  VkCommandBuffer swap_chain_command_buffers[MAX_SWAP_CHAIN_FRAMES];
  VkImage swap_chain_images[MAX_SWAP_CHAIN_FRAMES];
  VkImageView swap_chain_image_views[MAX_SWAP_CHAIN_FRAMES];
  VkFramebuffer swap_chain_framebuffers[MAX_SWAP_CHAIN_FRAMES];

  VkBuffer vertex_buffer;
  VkDeviceMemory vertex_buffer_memory;
  VkBuffer index_buffer;
  VkDeviceMemory index_buffer_memory;

  VkBuffer uniform_buffer;
  VkDeviceMemory uniform_buffers_memory;

  VkDescriptorSet descriptor_set[POST_PROCESS_PING_PONG];
};
#endif
#ifdef DIRECTX_12_API_SUPPORTED
struct SwapChainDirectX12 {
  IDXGISwapChain3 *swap_chain;

  ID3D12DescriptorHeap *rtv_descriptor_heap[MAX_SWAP_CHAIN_FRAMES];
  UINT rtv_descriptor_size[MAX_SWAP_CHAIN_FRAMES];
  D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle[MAX_SWAP_CHAIN_FRAMES];
  ID3D12Resource *render_targets[MAX_SWAP_CHAIN_FRAMES];
  ID3D12CommandAllocator *command_allocator[MAX_SWAP_CHAIN_FRAMES];
  ID3D12GraphicsCommandList *command_list[MAX_SWAP_CHAIN_FRAMES];

  ID3D12Resource *vertex_buffer;
  D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view;
  ID3D12Resource *index_buffer;
  D3D12_INDEX_BUFFER_VIEW index_buffer_view;

  ID3D12Resource *constant_buffer;  // Constant Buffer Resource

  ID3D12Fence *fence[MAX_FRAMES_IN_FLIGHT];
  HANDLE fence_event[MAX_FRAMES_IN_FLIGHT];
  UINT64 fence_value[MAX_FRAMES_IN_FLIGHT];
  UINT frame_index[MAX_FRAMES_IN_FLIGHT];
};
#endif

struct SwapChainCommon {
  uint_fast8_t descriptors;
  size_t current_frame;
  uint32_t image_index;
  uint_fast8_t supersample_scale;
  struct SwapchainExtent swap_chain_extent;

  struct BlitShader *blit_shader;
  struct Mesh blit_fullscreen_triangle;

#ifdef _WIN64
  HWND hwnd;
#endif
  union {
#ifdef VULKAN_API_SUPPORTED
    struct SwapChainVulkan swap_chain_vulkan;
#endif
#ifdef DIRECTX_12_API_SUPPORTED
    struct SwapChainDirectX12 swap_chain_directx12;
#endif
  };
};
