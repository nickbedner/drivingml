#include "mana/graphics/utilities/mesh/meshdirectx12.h"

u8 mesh_directx_12_init(struct MeshCommon* mesh_common, enum MESH_TYPE mesh_type, struct APICommon* api_common) {
  return 0;
}

void mesh_directx_12_delete(struct MeshCommon* mesh_common, struct APICommon* api_common) {
  //// Note: D3D12_VERTEX_BUFFER_VIEW and D3D12_INDEX_BUFFER_VIEW don't need explicit releases as they're just views
  // if (mesh_common->mesh_directx12.vertex_buffer) {
  //   mesh_common->mesh_directx12.vertex_buffer->lpVtbl->Release(mesh_common->mesh_directx12.vertex_buffer);
  //   mesh_common->mesh_directx12.vertex_buffer = NULL;
  // }
  //
  // if (mesh_common->mesh_directx12.index_buffer) {
  //  mesh_common->mesh_directx12.index_buffer->lpVtbl->Release(mesh_common->mesh_directx12.index_buffer);
  //  mesh_common->mesh_directx12.index_buffer = NULL;
  //}
}

void mesh_directx_12_generate_buffers(struct MeshCommon* mesh_common, struct APICommon* api_common) {
  // directx_12_graphics_utils_setup_vertex_buffer(&api_common->directx_12_api, mesh_common->vertices, &(mesh_common->mesh_directx12.vertex_buffer));
  // directx_12_graphics_utils_setup_index_buffer(&api_common->directx_12_api, mesh_common->indices, &(mesh_common->mesh_directx12.index_buffer));
}
