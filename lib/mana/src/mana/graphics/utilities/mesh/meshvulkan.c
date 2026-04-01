#include "mana/graphics/utilities/mesh/meshvulkan.h"

u8 mesh_vulkan_init(struct MeshCommon* mesh_common, enum MESH_TYPE mesh_type, struct APICommon* api_common) {
  return 0;
}

void mesh_vulkan_delete(struct MeshCommon* mesh_common, struct APICommon* api_common) {
  // vkDestroyBuffer(api_common->vulkan_api.device, mesh_common->mesh_vulkan.index_buffer, NULL);
  // vkFreeMemory(api_common->vulkan_api.device, mesh_common->mesh_vulkan.index_buffer_memory, NULL);
  //
  // vkDestroyBuffer(api_common->vulkan_api.device, mesh_common->mesh_vulkan.vertex_buffer, NULL);
  // vkFreeMemory(api_common->vulkan_api.device, mesh_common->mesh_vulkan.vertex_buffer_memory, NULL);
}

void mesh_vulkan_generate_buffers(struct MeshCommon* mesh_common, struct APICommon* api_common) {
  // vulkan_graphics_utils_setup_vertex_buffer(&api_common->vulkan_api, mesh_common->vertices, &(mesh_common->mesh_vulkan.vertex_buffer), &(mesh_common->mesh_vulkan.vertex_buffer_memory));
  // vulkan_graphics_utils_setup_index_buffer(&api_common->vulkan_api, mesh_common->indices, &(mesh_common->mesh_vulkan.index_buffer), &(mesh_common->mesh_vulkan.index_buffer_memory));
}
