#include "mana/graphics/entities/water/watervulkan.h"

uint_fast8_t water_vulkan_init(struct WaterCommon* water_common, struct APICommon* api_common, struct Shader* shader, struct Texture* texture) {
  vulkan_graphics_utils_setup_vertex_buffer(&api_common->vulkan_api, water_common->water_mesh->mesh_common.vertices, &water_common->water_vulkan.vertex_buffer, &water_common->water_vulkan.vertex_buffer_memory);
  vulkan_graphics_utils_setup_index_buffer(&api_common->vulkan_api, water_common->water_mesh->mesh_common.indices, &water_common->water_vulkan.index_buffer, &water_common->water_vulkan.index_buffer_memory);
  vulkan_graphics_utils_setup_uniform_buffer(&api_common->vulkan_api, sizeof(struct WaterVertexUniformBufferObject), &water_common->water_vulkan.vertex_uniform_buffer, &water_common->water_vulkan.vertex_uniform_memory);
  vulkan_graphics_utils_setup_uniform_buffer(&api_common->vulkan_api, sizeof(struct WaterFragmentUniformBufferObject), &water_common->water_vulkan.fragment_uniform_buffer, &water_common->water_vulkan.fragment_uniform_memory);

  VkDescriptorSetAllocateInfo alloc = {0};
  alloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  alloc.descriptorPool = water_common->shader->shader_common.shader_vulkan.descriptor_pool;
  alloc.descriptorSetCount = 1;
  alloc.pSetLayouts = &water_common->shader->shader_common.shader_vulkan.descriptor_set_layout;
  if (vkAllocateDescriptorSets(api_common->vulkan_api.device, &alloc, &water_common->water_vulkan.descriptor_set) != VK_SUCCESS) return 0;

  VkWriteDescriptorSet writes[3] = {0};

  vulkan_graphics_utils_setup_descriptor_buffer(writes, 0, &water_common->water_vulkan.descriptor_set, (VkDescriptorBufferInfo[]){vulkan_graphics_utils_setup_descriptor_buffer_info(sizeof(struct WaterVertexUniformBufferObject), &water_common->water_vulkan.vertex_uniform_buffer)});
  vulkan_graphics_utils_setup_descriptor_buffer(writes, 1, &water_common->water_vulkan.descriptor_set, (VkDescriptorBufferInfo[]){vulkan_graphics_utils_setup_descriptor_buffer_info(sizeof(struct WaterFragmentUniformBufferObject), &water_common->water_vulkan.fragment_uniform_buffer)});
  vulkan_graphics_utils_setup_descriptor_image(writes, 2, &water_common->water_vulkan.descriptor_set, (VkDescriptorImageInfo[]){vulkan_graphics_utils_setup_descriptor_image_info(&water_common->wave_texture->texture_common.texture_vulkan.texture_image_view, &water_common->wave_texture->texture_common.texture_vulkan.texture_sampler)});

  vkUpdateDescriptorSets(api_common->vulkan_api.device, 3, writes, 0, NULL);
  return WATER_SUCCESS;
}

void water_vulkan_delete(struct WaterCommon* water_common, struct APICommon* api_common) {
  VkDevice device = api_common->vulkan_api.device;
  vkDestroyBuffer(device, water_common->water_vulkan.index_buffer, NULL);
  vkFreeMemory(device, water_common->water_vulkan.index_buffer_memory, NULL);
  vkDestroyBuffer(device, water_common->water_vulkan.vertex_buffer, NULL);
  vkFreeMemory(device, water_common->water_vulkan.vertex_buffer_memory, NULL);
  vkDestroyBuffer(device, water_common->water_vulkan.vertex_uniform_buffer, NULL);
  vkFreeMemory(device, water_common->water_vulkan.vertex_uniform_memory, NULL);
  vkDestroyBuffer(device, water_common->water_vulkan.fragment_uniform_buffer, NULL);
  vkFreeMemory(device, water_common->water_vulkan.fragment_uniform_memory, NULL);
}

void water_vulkan_render(struct WaterCommon* water_common, struct GBufferCommon* gbuffer_common) {
  VkCommandBuffer cmd = gbuffer_common->gbuffer_vulkan.command_buffer;
  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, water_common->shader->shader_common.shader_vulkan.graphics_pipeline);

  VkBuffer vertex_buffers[] = {water_common->water_vulkan.vertex_buffer};
  VkDeviceSize offsets[] = {0};
  vkCmdSetViewport(cmd, 0, 1, &water_common->shader->shader_common.shader_vulkan.viewport);
  vkCmdSetScissor(cmd, 0, 1, &water_common->shader->shader_common.shader_vulkan.scissor);
  vkCmdBindVertexBuffers(cmd, 0, 1, vertex_buffers, offsets);
  vkCmdBindIndexBuffer(cmd, water_common->water_vulkan.index_buffer, 0, VK_INDEX_TYPE_UINT32);
  vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, water_common->shader->shader_common.shader_vulkan.pipeline_layout, 0, 1, &water_common->water_vulkan.descriptor_set, 0, NULL);
  vkCmdDrawIndexed(cmd, (uint32_t)water_common->water_mesh->mesh_common.indices->size, 1, 0, 0, 0);
}

void water_vulkan_update_uniforms(struct WaterCommon* water_common, struct APICommon* api_common, struct GBufferCommon* gbuffer_common, uint32_t width, uint32_t height) {
  VkDevice device = api_common->vulkan_api.device;
  void* data;

  struct WaterVertexUniformBufferObject vertex_ubo = {0};
  vertex_ubo.proj = gbuffer_common->projection_matrix;
  vertex_ubo.view = gbuffer_common->view_matrix;
  vertex_ubo.model = mat4_translate(MAT4_IDENTITY, water_common->position);
  vertex_ubo.model = mat4_mul(vertex_ubo.model, quaternion_to_mat4(quaternion_normalise(water_common->rotation)));
  vertex_ubo.model = mat4_scale(vertex_ubo.model, water_common->scale);

  vkMapMemory(device, water_common->water_vulkan.vertex_uniform_memory, 0, sizeof(vertex_ubo), 0, &data);
  memcpy(data, &vertex_ubo, sizeof(vertex_ubo));
  vkUnmapMemory(device, water_common->water_vulkan.vertex_uniform_memory);

  struct WaterFragmentUniformBufferObject frag_ubo = {0};
  frag_ubo.params0.x = water_common->time;
  frag_ubo.params0.y = 0.0f;  // test value: 0..4
  frag_ubo.params0.z = 0.0f;  // unused
  frag_ubo.params0.w = 0.0f;  // unused
  frag_ubo.params1 = (vec4){0};
  frag_ubo.params1.x = 3.0f;

  vkMapMemory(device, water_common->water_vulkan.fragment_uniform_memory, 0, sizeof(frag_ubo), 0, &data);
  memcpy(data, &frag_ubo, sizeof(frag_ubo));
  vkUnmapMemory(device, water_common->water_vulkan.fragment_uniform_memory);
}
