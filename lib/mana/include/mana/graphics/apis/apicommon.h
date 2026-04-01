#pragma once

#include "mana/core/corecommon.h"
#include "mana/graphics/graphicscommon.h"
#include "mana/math/advmath.h"
#include "mana/storage/storage.h"

#ifdef DIRECTX_12_API_SUPPORTED
#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi1_4.h>
#include <dxgi1_6.h>
#endif

#ifdef VULKAN_API_SUPPORTED
#ifdef _WIN64
#define VK_USE_PLATFORM_WIN32_KHR
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
#pragma clang diagnostic ignored "-Wnonportable-system-include-path"
#include <vulkan/vulkan.h>
#pragma clang diagnostic pop
#elif defined(__ANDROID__)
#define VK_USE_PLATFORM_ANDROID_KHR
#include <vulkan/vulkan.h>
#endif
#endif

#ifndef NDEBUG
#define ENABLE_DEBUG_CONTROLLER
#define ENABLE_VALIDATION_LAYERS
#endif

enum API_STATUS {
  API_SUCCESS = 0,
  API_ERROR,
  API_LAST_ERROR
};

enum API_SETUP_RENDERER_STATUS {
  API_SETUP_RENDERER_SUCCESS = 0,
  API_SETUP_RENDERER_ERROR,
  API_SETUP_RENDERER_LAST_ERROR
};

enum API_TYPE {
  API_NONE = 0,
#ifdef VULKAN_API_SUPPORTED
  API_VULKAN,
#endif
#ifdef DIRECTX_12_API_SUPPORTED
  API_DIRECTX_12,
#endif
#ifdef METAL_API_SUPPORTED
  API_APPLE_METAL,
#endif
  API_LAST
};

#ifdef DIRECTX_12_API_SUPPORTED
struct DirectX12API {
#ifdef ENABLE_DEBUG_CONTROLLER
  ID3D12Debug* debug_controller;
#endif
  ID3D12Device* device;
  IDXGIFactory4* factory;
  ID3D12CommandQueue* command_queue;
  ID3D12CommandAllocator* command_allocator;
  ID3D12GraphicsCommandList* command_list;
  ID3D12Fence* fence;  // The fence used for synchronization
  UINT64 fence_value;  // The current value of the fence
  HANDLE fence_event;  // Event used for CPU/GPU synchronization
};
#endif

#ifdef VULKAN_API_SUPPORTED
struct QueueFamilyIndices {
  u32 graphics_family;
  u32 present_family;
};

struct VulkanAPI {
  VkInstance instance;
  VkPhysicalDevice physical_device;
  VkDevice device;
  VkQueue graphics_queue;
  VkQueue present_queue;
  VkDebugUtilsMessengerEXT debug_messenger;
  VkCommandPool command_pool;
  struct QueueFamilyIndices indices;
};
#endif

struct APICommon {
  union {
#ifdef DIRECTX_12_API_SUPPORTED
    struct DirectX12API directx_12_api;
#endif
#ifdef VULKAN_API_SUPPORTED
    struct VulkanAPI vulkan_api;
#endif
  };

  enum API_TYPE api_type;

  b8 inverted_y;
  // Note: Padding for array alignment
  u8 _pad0[3];

  char asset_directory[MAX_LENGTH_OF_PATH];
};
