#pragma once

#include "mana/graphics/apis/api.h"
#include "mana/graphics/graphicscommon.h"
#include "mana/graphics/utilities/texturemanager/texturemanagercommon.h"

enum FilterType {
  FILTER_NEAREST = 0,
  FILTER_LINEAR
};

enum ModeType {
  MODE_REPEAT = 0,
  MODE_MIRRORED_REPEAT,
  MODE_CLAMP_TO_EDGE,
  MODE_CLAMP_TO_BORDER
};

enum MipType {
  MIP_NONE = 0,
  MIP_GENERATE,
  MIP_CUSTOM
};

// Note: RGB format isn't used because 24 bit not supported by DirectX and bad performance anyways because not aligned to 32 bits
enum FormatType {
  FORMAT_R8_UNORM = 0,
  FORMAT_R8G8B8A8_UNORM,
  FORMAT_R16_UNORM,
  FORMAT_R16G16B16A16_UNORM,
  FORMAT_R32_SFLOAT,
  FORMAT_R32G32B32A32_SFLOAT,
};

struct TextureSettings {
  enum FilterType filter_type;
  enum ModeType mode_type;
  enum FormatType format_type;

  enum MipType mip_type;
  uint32_t mip_count;

  // Does the image already have premultiplied alphas, 0: No 1: Yes
  bool premultiplied_alpha;

  bool anisotropy_enable;
  float max_anisotropy;
};

#ifdef VULKAN_API_SUPPORTED
struct TextureVulkan {
  VkImage texture_image;                // Raw image data in ram
  VkDeviceMemory texture_image_memory;  // Image data in GPU/device ram
  VkImageView texture_image_view;       // Texture format
  VkSampler texture_sampler;            // Image sampling settings
};
#endif

#ifdef DIRECTX_12_API_SUPPORTED
struct TextureDirectX12 {
  ID3D12Resource* texture_resource;             // the actual texture resource
  D3D12_CPU_DESCRIPTOR_HANDLE cpu_heap_handle;  // handle to the CPU-side descriptor heap
  D3D12_GPU_DESCRIPTOR_HANDLE gpu_heap_handle;  // handle to the GPU-side descriptor heap
};
#endif

struct TextureCommon {
  size_t id;
  wchar_t* name;
  wchar_t* type;
  wchar_t* path;
  uint32_t width;
  uint32_t height;

  struct TextureManagerCommon* texture_manager_common;

  union {
#ifdef VULKAN_API_SUPPORTED
    struct TextureVulkan texture_vulkan;
#endif
#ifdef DIRECTX_12_API_SUPPORTED
    struct TextureDirectX12 texture_directx12;
#endif
  };
};
