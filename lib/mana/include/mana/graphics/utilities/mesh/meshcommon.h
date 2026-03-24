#pragma once

#include "mana/graphics/graphicscommon.h"
#include "mana/graphics/utilities/texture/texture.h"
#include "mana/storage/storage.h"

enum MESH_TYPE {
  MESH_TYPE_SPRITE = 0,
  MESH_TYPE_QUAD,
  MESH_TYPE_TRIANGLE,
  MESH_TYPE_MODEL,
  MESH_TYPE_MODEL_STATIC,
  MESH_TYPE_DUAL_CONTOURING,
  MESH_TYPE_MANIFOLD_DUAL_CONTOURING,
  MESH_TYPE_GRASS
};

struct VertexSprite {
  vec3 position;
  vec2 tex_coord;
};

struct VertexQuad {
  vec3 position;
  vec2 tex_coord;
};

struct VertexTriangle {
  vec3 position;
};

struct VertexModel {
  vec3 position;
  vec3 normal;
  vec2 tex_coord;
  vec3 color;
  ivec3 joints_ids;
  vec3 weights;
};

struct VertexModelStatic {
  vec3 position;
  vec3 normal;
  vec2 tex_coord;
  vec3 color;
};

struct VertexDualContouring {
  vec3 position;
  vec3 normal;
};

struct VertexManifoldDualContouring {
  vec3 position;
  vec3 color;
  vec3 normal1;
  vec3 normal2;
};

struct VertexGrass {
  vec4 position_color;
};

#ifdef VULKAN_API_SUPPORTED
// struct MeshVulkan {
//   VkBuffer vertex_buffer;
//   VkDeviceMemory vertex_buffer_memory;
//   VkBuffer index_buffer;
//   VkDeviceMemory index_buffer_memory;
// };

static inline void mesh_get_attribute_descriptions(enum MESH_TYPE mesh_type, VkVertexInputAttributeDescription* attribute_descriptions) {
  switch (mesh_type) {
    case (MESH_TYPE_SPRITE): {
      attribute_descriptions[0].binding = 0;
      attribute_descriptions[0].location = 0;
      attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
      attribute_descriptions[0].offset = offsetof(struct VertexSprite, position);

      attribute_descriptions[1].binding = 0;
      attribute_descriptions[1].location = 1;
      attribute_descriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
      attribute_descriptions[1].offset = offsetof(struct VertexSprite, tex_coord);
      break;
    }
    case (MESH_TYPE_QUAD): {
      attribute_descriptions[0].binding = 0;
      attribute_descriptions[0].location = 0;
      attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
      attribute_descriptions[0].offset = offsetof(struct VertexQuad, position);

      attribute_descriptions[1].binding = 0;
      attribute_descriptions[1].location = 1;
      attribute_descriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
      attribute_descriptions[1].offset = offsetof(struct VertexQuad, tex_coord);
      break;
    }
    case (MESH_TYPE_TRIANGLE): {
      attribute_descriptions[0].binding = 0;
      attribute_descriptions[0].location = 0;
      attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
      attribute_descriptions[0].offset = offsetof(struct VertexTriangle, position);
      break;
    }
    case (MESH_TYPE_MODEL): {
      attribute_descriptions[0].binding = 0;
      attribute_descriptions[0].location = 0;
      attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
      attribute_descriptions[0].offset = offsetof(struct VertexModel, position);

      attribute_descriptions[1].binding = 0;
      attribute_descriptions[1].location = 1;
      attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
      attribute_descriptions[1].offset = offsetof(struct VertexModel, normal);

      attribute_descriptions[2].binding = 0;
      attribute_descriptions[2].location = 2;
      attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
      attribute_descriptions[2].offset = offsetof(struct VertexModel, tex_coord);

      attribute_descriptions[3].binding = 0;
      attribute_descriptions[3].location = 3;
      attribute_descriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
      attribute_descriptions[3].offset = offsetof(struct VertexModel, color);

      attribute_descriptions[4].binding = 0;
      attribute_descriptions[4].location = 4;
      attribute_descriptions[4].format = VK_FORMAT_R32G32B32_SINT;
      attribute_descriptions[4].offset = offsetof(struct VertexModel, joints_ids);

      attribute_descriptions[5].binding = 0;
      attribute_descriptions[5].location = 5;
      attribute_descriptions[5].format = VK_FORMAT_R32G32B32_SFLOAT;
      attribute_descriptions[5].offset = offsetof(struct VertexModel, weights);
      break;
    }
    case (MESH_TYPE_MODEL_STATIC): {
      attribute_descriptions[0].binding = 0;
      attribute_descriptions[0].location = 0;
      attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
      attribute_descriptions[0].offset = offsetof(struct VertexModelStatic, position);

      attribute_descriptions[1].binding = 0;
      attribute_descriptions[1].location = 1;
      attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
      attribute_descriptions[1].offset = offsetof(struct VertexModelStatic, normal);

      attribute_descriptions[2].binding = 0;
      attribute_descriptions[2].location = 2;
      attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
      attribute_descriptions[2].offset = offsetof(struct VertexModelStatic, tex_coord);

      attribute_descriptions[3].binding = 0;
      attribute_descriptions[3].location = 3;
      attribute_descriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
      attribute_descriptions[3].offset = offsetof(struct VertexModelStatic, color);
      break;
    }
    case (MESH_TYPE_DUAL_CONTOURING): {
      attribute_descriptions[0].binding = 0;
      attribute_descriptions[0].location = 0;
      attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
      attribute_descriptions[0].offset = offsetof(struct VertexDualContouring, position);

      attribute_descriptions[1].binding = 0;
      attribute_descriptions[1].location = 1;
      attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
      attribute_descriptions[1].offset = offsetof(struct VertexDualContouring, normal);
      break;
    }
    case (MESH_TYPE_MANIFOLD_DUAL_CONTOURING): {
      attribute_descriptions[0].binding = 0;
      attribute_descriptions[0].location = 0;
      attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
      attribute_descriptions[0].offset = offsetof(struct VertexManifoldDualContouring, position);

      attribute_descriptions[1].binding = 0;
      attribute_descriptions[1].location = 1;
      attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
      attribute_descriptions[1].offset = offsetof(struct VertexManifoldDualContouring, color);

      attribute_descriptions[2].binding = 0;
      attribute_descriptions[2].location = 2;
      attribute_descriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
      attribute_descriptions[2].offset = offsetof(struct VertexManifoldDualContouring, normal1);

      attribute_descriptions[3].binding = 0;
      attribute_descriptions[3].location = 3;
      attribute_descriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
      attribute_descriptions[3].offset = offsetof(struct VertexManifoldDualContouring, normal2);
      break;
    }
    case (MESH_TYPE_GRASS): {
      attribute_descriptions[0].binding = 0;
      attribute_descriptions[0].location = 0;
      attribute_descriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
      attribute_descriptions[0].offset = offsetof(struct VertexGrass, position_color);
      break;
    }
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcovered-switch-default"
    default: {
      __builtin_unreachable();
    }
#pragma clang diagnostic pop
  }
}
#endif

#ifdef DIRECTX_12_API_SUPPORTED
// struct MeshDirectX12 {
//   //ID3D12Resource *vertex_buffer;
//   //D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view;
//   //ID3D12Resource *index_buffer;
//   //D3D12_INDEX_BUFFER_VIEW index_buffer_view;
// };

static inline uint32_t mesh_get_input_layout(enum MESH_TYPE mesh_type, D3D12_INPUT_ELEMENT_DESC* inputElementDescs) {
  uint32_t num_attributes = 0;  // This will be used to determine how many attributes were added for the mesh type.

  switch (mesh_type) {
    case (MESH_TYPE_SPRITE): {
      inputElementDescs[num_attributes++] = (D3D12_INPUT_ELEMENT_DESC){"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(struct VertexSprite, position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0};
      inputElementDescs[num_attributes++] = (D3D12_INPUT_ELEMENT_DESC){"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(struct VertexSprite, tex_coord), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0};
      break;
    }
    case (MESH_TYPE_QUAD): {
      inputElementDescs[num_attributes++] = (D3D12_INPUT_ELEMENT_DESC){"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(struct VertexQuad, position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0};
      inputElementDescs[num_attributes++] = (D3D12_INPUT_ELEMENT_DESC){"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(struct VertexQuad, tex_coord), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0};
      break;
    }
    case (MESH_TYPE_TRIANGLE): {
      inputElementDescs[num_attributes++] = (D3D12_INPUT_ELEMENT_DESC){"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(struct VertexTriangle, position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0};
      break;
    }
    case (MESH_TYPE_MODEL): {
      // Assuming the Model has multiple attributes
      inputElementDescs[num_attributes++] = (D3D12_INPUT_ELEMENT_DESC){"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(struct VertexModel, position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0};
      inputElementDescs[num_attributes++] = (D3D12_INPUT_ELEMENT_DESC){"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(struct VertexModel, normal), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0};
      inputElementDescs[num_attributes++] = (D3D12_INPUT_ELEMENT_DESC){"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(struct VertexModel, tex_coord), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0};
      inputElementDescs[num_attributes++] = (D3D12_INPUT_ELEMENT_DESC){"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(struct VertexModel, color), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0};
      inputElementDescs[num_attributes++] = (D3D12_INPUT_ELEMENT_DESC){"JOINTS", 0, DXGI_FORMAT_R32G32B32_SINT, 0, offsetof(struct VertexModel, joints_ids), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0};
      inputElementDescs[num_attributes++] = (D3D12_INPUT_ELEMENT_DESC){"WEIGHTS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(struct VertexModel, weights), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0};
      break;
    }
    case (MESH_TYPE_MODEL_STATIC): {
      inputElementDescs[num_attributes++] = (D3D12_INPUT_ELEMENT_DESC){"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(struct VertexModelStatic, position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0};
      inputElementDescs[num_attributes++] = (D3D12_INPUT_ELEMENT_DESC){"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(struct VertexModelStatic, normal), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0};
      inputElementDescs[num_attributes++] = (D3D12_INPUT_ELEMENT_DESC){"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(struct VertexModelStatic, tex_coord), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0};
      inputElementDescs[num_attributes++] = (D3D12_INPUT_ELEMENT_DESC){"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(struct VertexModelStatic, color), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0};
      break;
    }
    case (MESH_TYPE_DUAL_CONTOURING): {
      inputElementDescs[num_attributes++] = (D3D12_INPUT_ELEMENT_DESC){"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(struct VertexDualContouring, position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0};
      inputElementDescs[num_attributes++] = (D3D12_INPUT_ELEMENT_DESC){"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(struct VertexDualContouring, normal), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0};
      break;
    }
    case (MESH_TYPE_MANIFOLD_DUAL_CONTOURING): {
      inputElementDescs[num_attributes++] = (D3D12_INPUT_ELEMENT_DESC){"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(struct VertexManifoldDualContouring, position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0};
      inputElementDescs[num_attributes++] = (D3D12_INPUT_ELEMENT_DESC){"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(struct VertexManifoldDualContouring, color), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0};
      inputElementDescs[num_attributes++] = (D3D12_INPUT_ELEMENT_DESC){"NORMAL1", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(struct VertexManifoldDualContouring, normal1), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0};
      inputElementDescs[num_attributes++] = (D3D12_INPUT_ELEMENT_DESC){"NORMAL2", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(struct VertexManifoldDualContouring, normal2), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0};
      break;
    }
    case (MESH_TYPE_GRASS): {
      inputElementDescs[num_attributes++] = (D3D12_INPUT_ELEMENT_DESC){"POSITION_COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(struct VertexGrass, position_color), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0};
      break;
    }
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcovered-switch-default"
    default: {
      __builtin_unreachable();
    }
#pragma clang diagnostic pop
  }

  return num_attributes;  // Return the number of attributes added, so the caller knows how many were set up.
}
#endif

struct MeshCommon {
  enum MESH_TYPE mesh_type;
  uint32_t mesh_memory_size;
  struct Vector* vertices;
  struct Vector* indices;
  struct Vector* textures;

  //  union {
  // #ifdef VULKAN_API_SUPPORTED
  //    struct MeshVulkan mesh_vulkan;
  // #endif
  // #ifdef DIRECTX_12_API_SUPPORTED
  //    struct MeshDirectX12 mesh_directx12;
  // #endif
  //  };
};
