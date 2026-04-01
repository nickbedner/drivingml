#pragma once

#include <mana/graphics/apis/api.h>

#include "mana/graphics/graphicscommon.h"
#include "mana/graphics/render/gbuffer/gbuffer.h"
#include "mana/graphics/shaders/shader.h"
#include "mana/graphics/utilities/mesh/mesh.h"
#include "mana/graphics/utilities/texture/texture.h"

struct WaterVertexUniformBufferObject {
  alignas(16) mat4 model;
  alignas(16) mat4 view;
  alignas(16) mat4 proj;
};

struct WaterFragmentUniformBufferObject {
  // time, original_w, original_h, current_w
  alignas(16) vec4 params0;
  // current_h, unused, unused, unused
  alignas(16) vec4 params1;
};

enum {
  WATER_SUCCESS = 1
};

#ifdef VULKAN_API_SUPPORTED
struct WaterVulkan {
  VkBuffer vertex_buffer;
  VkDeviceMemory vertex_buffer_memory;

  VkBuffer index_buffer;
  VkDeviceMemory index_buffer_memory;

  VkBuffer vertex_uniform_buffer;
  VkDeviceMemory vertex_uniform_memory;

  VkBuffer fragment_uniform_buffer;
  VkDeviceMemory fragment_uniform_memory;

  VkDescriptorSet descriptor_set;
};
#endif

#ifdef DIRECTX_12_API_SUPPORTED
struct WaterDirectX12 {
  ID3D12Resource* vertex_buffer;
  D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view;

  ID3D12Resource* index_buffer;
  D3D12_INDEX_BUFFER_VIEW index_buffer_view;

  ID3D12Resource* vertex_constant_buffer;
  ID3D12Resource* fragment_constant_buffer;
};
#endif

struct WaterCommon {
  struct Shader* shader;
  struct Mesh* water_mesh;
  struct Texture* wave_texture;

  quat rotation;
  vec3 position;
  vec3 scale;

  r32 width;
  r32 height;
  r32 time;
  // Interal padding to ensure 16 byte alignment of uniform buffer data
  r32 _pad0;

  union {
#ifdef VULKAN_API_SUPPORTED
    struct WaterVulkan water_vulkan;
#endif
#ifdef DIRECTX_12_API_SUPPORTED
    struct WaterDirectX12 water_directx12;
#endif
  };
};
