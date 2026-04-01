#include "mana/core/graphics/utilities/mesh/mesh.h"

u8 mesh_init(struct Mesh* mesh, enum MESH_TYPE mesh_type, struct APICommon* api_common) {
#ifdef VULKAN_API_SUPPORTED
  if (api_common->api_type == API_VULKAN)
    mesh->mesh_func = MESH_VULKAN;
#endif
#ifdef DIRECTX_12_API_SUPPORTED
  if (api_common->api_type == API_DIRECTX_12)
    mesh->mesh_func = MESH_DIRECTX12;
#endif

  u32 mesh_memory_size = 0;
  switch (mesh_type) {
    case MESH_TYPE_SPRITE: {
      mesh_memory_size = sizeof(struct VertexSprite);
      break;
    }
    case MESH_TYPE_QUAD: {
      mesh_memory_size = sizeof(struct VertexQuad);
      break;
    }
    case MESH_TYPE_TRIANGLE: {
      mesh_memory_size = sizeof(struct VertexTriangle);
      break;
    }
    case MESH_TYPE_MODEL: {
      mesh_memory_size = sizeof(struct VertexModel);
      break;
    }
    case MESH_TYPE_MODEL_STATIC: {
      mesh_memory_size = sizeof(struct VertexModelStatic);
      break;
    }
    case MESH_TYPE_DUAL_CONTOURING: {
      mesh_memory_size = sizeof(struct VertexDualContouring);
      break;
    }
    case MESH_TYPE_MANIFOLD_DUAL_CONTOURING: {
      mesh_memory_size = sizeof(struct VertexManifoldDualContouring);
      break;
    }
    case MESH_TYPE_GRASS: {
      mesh_memory_size = sizeof(struct VertexGrass);
      break;
    }
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcovered-switch-default"
    default: {
      __builtin_unreachable();
    }
#pragma clang diagnostic pop
  }

  mesh->mesh_common.mesh_type = mesh_type;
  mesh->mesh_common.mesh_memory_size = mesh_memory_size;

  mesh->mesh_common.vertices = (struct Vector*)calloc(1, sizeof(struct Vector));
  vector_init(mesh->mesh_common.vertices, mesh_memory_size);

  mesh->mesh_common.indices = (struct Vector*)calloc(1, sizeof(struct Vector));
  vector_init(mesh->mesh_common.indices, sizeof(u32));

  mesh->mesh_func.mesh_init(&(mesh->mesh_common), mesh_type, api_common);

  return 0;
}

void mesh_delete(struct Mesh* mesh, struct APICommon* api_common) {
  mesh->mesh_func.mesh_delete(&(mesh->mesh_common), api_common);

  vector_delete(mesh->mesh_common.vertices);
  free(mesh->mesh_common.vertices);

  vector_delete(mesh->mesh_common.indices);
  free(mesh->mesh_common.indices);
}

u32 mesh_get_memory_size(enum MESH_TYPE mesh_type) {
  u32 mesh_memory_size = 0;
  switch (mesh_type) {
    case MESH_TYPE_SPRITE: {
      return mesh_memory_size = sizeof(struct VertexSprite);
    }
    case MESH_TYPE_QUAD: {
      return mesh_memory_size = sizeof(struct VertexQuad);
    }
    case MESH_TYPE_TRIANGLE: {
      return mesh_memory_size = sizeof(struct VertexTriangle);
    }
    case MESH_TYPE_MODEL: {
      return mesh_memory_size = sizeof(struct VertexModel);
    }
    case MESH_TYPE_MODEL_STATIC: {
      return mesh_memory_size = sizeof(struct VertexModelStatic);
    }
    case MESH_TYPE_DUAL_CONTOURING: {
      return mesh_memory_size = sizeof(struct VertexDualContouring);
    }
    case MESH_TYPE_MANIFOLD_DUAL_CONTOURING: {
      return mesh_memory_size = sizeof(struct VertexManifoldDualContouring);
    }
    case MESH_TYPE_GRASS: {
      return mesh_memory_size = sizeof(struct VertexGrass);
    }
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcovered-switch-default"
    default: {
      __builtin_unreachable();
    }
#pragma clang diagnostic pop
  }
}

void mesh_generate_buffers(struct Mesh* mesh, struct APICommon* api_common) {
  mesh->mesh_func.mesh_generate_buffers(&(mesh->mesh_common), api_common);
}

void mesh_clear(struct Mesh* mesh) {
  mesh_clear_vertices(mesh);
  mesh_clear_indices(mesh);
}

void mesh_clear_vertices(struct Mesh* mesh) {
  vector_clear(mesh->mesh_common.vertices);
}

void mesh_clear_indices(struct Mesh* mesh) {
  vector_clear(mesh->mesh_common.indices);
}

void mesh_assign_vertex(struct Mesh* mesh, void* vertex) {
  vector_push_back(mesh->mesh_common.vertices, vertex);
}

void mesh_assign_indice(struct Mesh* mesh, u32 indice) {
  vector_push_back(mesh->mesh_common.indices, &indice);
}

void mesh_fullscreen_triangle(struct Mesh* mesh) {
  mesh_assign_vertex(mesh, (void*)&(struct VertexTriangle){.position = {.x = -1.0f, .y = 1.0f, .z = 0.0f}});
  mesh_assign_vertex(mesh, (void*)&(struct VertexTriangle){.position = {.x = -1.0f, .y = -2.0f, .z = 0.0f}});
  mesh_assign_vertex(mesh, (void*)&(struct VertexTriangle){.position = {.x = 2.0f, .y = 1.0f, .z = 0.0f}});

  mesh_assign_indice(mesh, 0);
  mesh_assign_indice(mesh, 1);
  mesh_assign_indice(mesh, 2);
}
