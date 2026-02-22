#pragma once

#include <mana/graphics/apis/api.h>

#include "mana/graphics/graphicscommon.h"
#include "mana/graphics/render/gbuffer/gbuffer.h"
#include "mana/graphics/shaders/shader.h"
#include "mana/graphics/utilities/mesh/mesh.h"
#include "mana/graphics/utilities/texture/texture.h"


struct SpriteAnimationUniformBufferObject {
  alignas(16) mat4 model;
  alignas(16) mat4 view;
  alignas(16) mat4 proj;
  alignas(16) vec4 frame_pos_xy_direction_z;
};

enum {
  SPRITE_ANIMATION_SUCCESS = 1
};

static const float SPRITE_ANIMATION_FORWARD = 1.0f;
static const float SPRITE_ANIMATION_BACKWARD = -1.0;

#ifdef VULKAN_API_SUPPORTED
struct SpriteAnimationVulkan {
  VkBuffer vertex_buffer;
  VkDeviceMemory vertex_buffer_memory;
  VkBuffer index_buffer;
  VkDeviceMemory index_buffer_memory;
  VkBuffer uniform_buffer;
  VkDeviceMemory uniform_buffers_memory;

  VkDescriptorSet *descriptor_set;
};
#endif
#ifdef DIRECTX_12_API_SUPPORTED
struct SpriteAnimationDirectX12 {
  ID3D12Resource *vertex_buffer;                // Vertex Buffer Resource
  D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view;  // View into the Vertex Buffer
  ID3D12Resource *index_buffer;                 // Index Buffer Resource
  D3D12_INDEX_BUFFER_VIEW index_buffer_view;    // View into the Index Buffer
  ID3D12Resource *constant_buffer;              // Constant Buffer Resource
};
#endif

struct SpriteAnimationCommon {
  struct Shader *shader;
  struct Mesh *image_mesh;
  struct Texture *image_texture;

  vec3 position;
  vec3 scale;
  quat rotation;
  float width;
  float height;

  vec3 frame_pos;
  int32_t total_frames;
  int32_t current_frame;
  float total_animation_length;
  float frame_length;
  float current_animation_time;
  float padding;
  float direction;
  bool animate;
  bool loop;

  size_t sprite_num;

  union {
#ifdef VULKAN_API_SUPPORTED
    struct SpriteAnimationVulkan sprite_animation_vulkan;
#endif
#ifdef DIRECTX_12_API_SUPPORTED
    struct SpriteAnimationDirectX12 sprite_animation_directx12;
#endif
  };
};
