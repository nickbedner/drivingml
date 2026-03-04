#include "mana/graphics/entities/sprite/spriteanimationvulkan.h"

uint_fast8_t sprite_animation_vulkan_init(struct SpriteAnimationCommon* sprite_animation_common, struct APICommon* api_common, struct Shader* shader, struct Texture* texture) {
  vulkan_graphics_utils_setup_vertex_buffer(&api_common->vulkan_api, sprite_animation_common->image_mesh->mesh_common.vertices, &sprite_animation_common->sprite_animation_vulkan.vertex_buffer, &sprite_animation_common->sprite_animation_vulkan.vertex_buffer_memory);
  vulkan_graphics_utils_setup_index_buffer(&api_common->vulkan_api, sprite_animation_common->image_mesh->mesh_common.indices, &sprite_animation_common->sprite_animation_vulkan.index_buffer, &sprite_animation_common->sprite_animation_vulkan.index_buffer_memory);
  vulkan_graphics_utils_setup_uniform_buffer(&api_common->vulkan_api, sizeof(struct SpriteAnimationUniformBufferObject), &(sprite_animation_common->sprite_animation_vulkan.uniform_buffer), &(sprite_animation_common->sprite_animation_vulkan.uniform_buffers_memory));

  VkWriteDescriptorSet dcs[2] = {0};
  vulkan_graphics_utils_setup_descriptor_buffer(dcs, 0, sprite_animation_common->sprite_animation_vulkan.descriptor_set, (VkDescriptorBufferInfo[]){vulkan_graphics_utils_setup_descriptor_buffer_info(sizeof(struct SpriteAnimationUniformBufferObject), &sprite_animation_common->sprite_animation_vulkan.uniform_buffer)});
  vulkan_graphics_utils_setup_descriptor_image(dcs, 1, sprite_animation_common->sprite_animation_vulkan.descriptor_set, (VkDescriptorImageInfo[]){vulkan_graphics_utils_setup_descriptor_image_info(&sprite_animation_common->image_texture->texture_common.texture_vulkan.texture_image_view, &(sprite_animation_common->image_texture->texture_common.texture_vulkan.texture_sampler))});
  vkUpdateDescriptorSets(api_common->vulkan_api.device, 2, dcs, 0, NULL);

  return 0;
}

void sprite_animation_vulkan_delete(struct SpriteAnimationCommon* sprite_animation_common, struct APICommon* api_common) {
  vkDestroyBuffer(api_common->vulkan_api.device, sprite_animation_common->sprite_animation_vulkan.index_buffer, NULL);
  vkFreeMemory(api_common->vulkan_api.device, sprite_animation_common->sprite_animation_vulkan.index_buffer_memory, NULL);

  vkDestroyBuffer(api_common->vulkan_api.device, sprite_animation_common->sprite_animation_vulkan.vertex_buffer, NULL);
  vkFreeMemory(api_common->vulkan_api.device, sprite_animation_common->sprite_animation_vulkan.vertex_buffer_memory, NULL);

  vkDestroyBuffer(api_common->vulkan_api.device, sprite_animation_common->sprite_animation_vulkan.uniform_buffer, NULL);
  vkFreeMemory(api_common->vulkan_api.device, sprite_animation_common->sprite_animation_vulkan.uniform_buffers_memory, NULL);
}

void sprite_animation_vulkan_render(struct SpriteAnimationCommon* sprite_animation_common, struct GBufferCommon* gbuffer_common) {
  vkCmdBindPipeline(gbuffer_common->gbuffer_vulkan.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, sprite_animation_common->shader->shader_common.shader_vulkan.graphics_pipeline);

  VkBuffer vertex_buffers[] = {sprite_animation_common->sprite_animation_vulkan.vertex_buffer};
  VkDeviceSize offsets[] = {0};
  vkCmdSetViewport(gbuffer_common->gbuffer_vulkan.command_buffer, 0, 1, &(sprite_animation_common->shader->shader_common.shader_vulkan.viewport));
  vkCmdSetScissor(gbuffer_common->gbuffer_vulkan.command_buffer, 0, 1, &(sprite_animation_common->shader->shader_common.shader_vulkan.scissor));
  vkCmdBindVertexBuffers(gbuffer_common->gbuffer_vulkan.command_buffer, 0, 1, vertex_buffers, offsets);
  vkCmdBindIndexBuffer(gbuffer_common->gbuffer_vulkan.command_buffer, sprite_animation_common->sprite_animation_vulkan.index_buffer, 0, VK_INDEX_TYPE_UINT32);
  vkCmdBindDescriptorSets(gbuffer_common->gbuffer_vulkan.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, sprite_animation_common->shader->shader_common.shader_vulkan.pipeline_layout, 0, 1, sprite_animation_common->sprite_animation_vulkan.descriptor_set, 0, NULL);
  vkCmdDrawIndexed(gbuffer_common->gbuffer_vulkan.command_buffer, (uint32_t)sprite_animation_common->image_mesh->mesh_common.indices->size, 1, 0, 0, 0);
}

void sprite_animation_vulkan_update_uniforms(struct SpriteAnimationCommon* sprite_animation_common, struct APICommon* api_common, struct GBufferCommon* gbuffer_common) {
  struct SpriteAnimationUniformBufferObject ubos = {0};
  ubos.proj = gbuffer_common->projection_matrix;
  ubos.view = gbuffer_common->view_matrix;

  ubos.model = mat4_translate(MAT4_IDENTITY, sprite_animation_common->position);
  ubos.model = mat4_mul(ubos.model, quaternion_to_mat4(quaternion_normalise(sprite_animation_common->rotation)));
  ubos.model = mat4_scale(ubos.model, sprite_animation_common->scale);

  ubos.frame_pos_xy_direction_z.x = sprite_animation_common->frame_pos.x;
  ubos.frame_pos_xy_direction_z.y = sprite_animation_common->frame_pos.y;
  ubos.frame_pos_xy_direction_z.z = sprite_animation_common->direction;
  ubos.frame_pos_xy_direction_z.w = sprite_animation_common->padding;

  void* data;
  vkMapMemory(api_common->vulkan_api.device, sprite_animation_common->sprite_animation_vulkan.uniform_buffers_memory, 0, sizeof(struct SpriteAnimationUniformBufferObject), 0, &data);
  memcpy(data, &ubos, sizeof(struct SpriteAnimationUniformBufferObject));
  vkUnmapMemory(api_common->vulkan_api.device, sprite_animation_common->sprite_animation_vulkan.uniform_buffers_memory);
}
