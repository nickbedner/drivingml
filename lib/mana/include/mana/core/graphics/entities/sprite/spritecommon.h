#pragma once

#include <mana/core/graphics/apis/api.h>

#include "mana/core/graphics/graphicscommon.h"
#include "mana/core/graphics/render/gbuffer/gbuffer.h"
#include "mana/core/graphics/shaders/shader.h"
#include "mana/core/graphics/utilities/mesh/mesh.h"
#include "mana/core/graphics/utilities/texture/texture.h"

struct SpriteUniformBufferObject {
  alignas(16) mat4 model;
  alignas(16) mat4 view;
  alignas(16) mat4 proj;
};

struct SpritePushConstants {
  i32 frame_layer;
};

enum {
  SPRITE_SUCCESS = 1
};

#ifdef VULKAN_API_SUPPORTED
struct SpriteVulkan {
  VkBuffer vertex_buffer;
  VkDeviceMemory vertex_buffer_memory;
  VkBuffer index_buffer;
  VkDeviceMemory index_buffer_memory;
  VkBuffer uniform_buffer;
  VkDeviceMemory uniform_buffers_memory;

  VkDescriptorSet* descriptor_set;
};
#endif

#ifdef DIRECTX_12_API_SUPPORTED
struct SpriteDirectX12 {
  ID3D12Resource* vertex_buffer;                // Vertex Buffer Resource
  D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view;  // View into the Vertex Buffer
  ID3D12Resource* index_buffer;                 // Index Buffer Resource
  D3D12_INDEX_BUFFER_VIEW index_buffer_view;    // View into the Index Buffer
  ID3D12Resource* constant_buffer;              // Constant Buffer Resource
};

#endif
struct SpriteCommon {
  struct Shader* shader;
  struct Mesh* image_mesh;
  struct Texture* image_texture;

  // Transform (likely 16-byte aligned)
  quat rotation;
  vec3 position;
  vec3 scale;

  size_t sprite_num;

  union {
#ifdef VULKAN_API_SUPPORTED
    struct SpriteVulkan sprite_vulkan;
#endif
#ifdef DIRECTX_12_API_SUPPORTED
    struct SpriteDirectX12 sprite_directx12;
#endif
  };

  // Small stuff last
  r32 width;
  r32 height;
  u32 frame_layer;
};
