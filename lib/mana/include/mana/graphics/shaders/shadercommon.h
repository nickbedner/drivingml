#pragma once

#include <mana/graphics/apis/api.h>

#include "mana/core/corecommon.h"
#include "mana/graphics/apis/api.h"
#include "mana/graphics/apis/apivulkan.h"
#include "mana/graphics/graphicscommon.h"
#include "mana/graphics/render/gbuffer/gbuffercommon.h"
#include "mana/graphics/render/swapchain/swapchaincommon.h"
#include "mana/graphics/utilities/mesh/mesh.h"
#include "mana/utilities/fileio.h"

#define MAX_PATH_LENGTH 2048

#define SHADER_ATTACHMENT_LIMIT 8

enum SHADER_STAGE {
  SHADER_STAGE_VERTEX = 0,
  SHADER_STAGE_FRAGMENT,
  SHADER_STAGE_VERTEX_AND_FRAGMENT
};

enum SHADER_FRONT_FACE {
  SHADER_FRONT_FACE_CLOCKWISE = 0,
  SHADER_FRONT_FACE_COUNTER_CLOCKWISE
};

enum SHADER_CULL_MODE {
  SHADER_CULL_MODE_NONE = 0,
  SHADER_CULL_MODE_FRONT_BIT,
  SHADER_CULL_MODE_BACK_BIT,
  SHADER_CULL_MODE_FRONT_AND_BACK_BIT
};

enum SHADER_RENDER_TARGET_FORMAT {
  SHADER_RENDER_TARGET_FORMAT_R8G8B8A8_UNORM = 0,
  SHADER_RENDER_TARGET_FORMAT_R11G11B10_FLOAT,
  SHADER_RENDER_TARGET_FORMAT_R16G16B16A16_FLOAT,
  SHADER_RENDER_TARGET_FORMAT_R32G32B32A32_FLOAT,
  SHADER_RENDER_TARGET_FORMAT_D32_FLOAT
};

struct ShaderAttributeSettings {
  enum SHADER_STAGE shader_stage;
  uint32_t shader_position;
};

struct ShaderSettings {
  uint_fast32_t descriptors;
  const char* vertex_shader;
  const char* fragment_shader;
  bool depth_test;
  bool depth_write;
  bool supersampled;
  bool blend;
  uint32_t mesh_memory_size;
  uint_fast32_t num_msaa_samples;
  uint_fast8_t color_attachments;
  uint_fast8_t vertex_attributes;
  uint_fast8_t uniforms_constants;
  struct ShaderAttributeSettings uniform_constant_state[SHADER_ATTACHMENT_LIMIT];
  uint_fast8_t texture_samples;
  struct ShaderAttributeSettings texture_sample_state[SHADER_ATTACHMENT_LIMIT];
  uint_fast8_t render_targets;
  enum SHADER_RENDER_TARGET_FORMAT render_target_format[SHADER_ATTACHMENT_LIMIT];
  enum SHADER_FRONT_FACE front_face;
  enum SHADER_CULL_MODE cull_mode;
  enum MESH_TYPE mesh_type;
  void* extra_data;
};

struct ShaderSettingsCompute {
  wchar_t* compute_shader;
  void* render_pass;
};

#ifdef VULKAN_API_SUPPORTED
struct ShaderVulkan {
  VkPipelineLayout pipeline_layout;
  VkPipeline graphics_pipeline;
  VkDescriptorPool descriptor_pool;
  VkDescriptorSetLayout descriptor_set_layout;
  VkViewport viewport;
  VkRect2D scissor;
};
#endif

#ifdef DIRECTX_12_API_SUPPORTED
struct ShaderDirectX12 {
  ID3D12PipelineState* pipeline_state;
  ID3D12RootSignature* root_signature;
  ID3DBlob* vertex_shader_blob;
  ID3DBlob* fragment_shader_blob;
  ID3D12DescriptorHeap* sampler_heap;
  D3D12_CPU_DESCRIPTOR_HANDLE sampler_handle_cpu;
  D3D12_GPU_DESCRIPTOR_HANDLE sampler_handle_gpu;
  D3D12_VIEWPORT viewport;
  D3D12_RECT scissor_rect;
};
#endif

struct ShaderCommon {
  union {
    struct ShaderSettings shader_settings;
    struct ShaderSettingsCompute shader_settings_compute;
  };
  union {
#ifdef VULKAN_API_SUPPORTED
    struct ShaderVulkan shader_vulkan;
#endif
#ifdef DIRECTX_12_API_SUPPORTED
    struct ShaderDirectX12 shader_directx12;
#endif
  };
};
