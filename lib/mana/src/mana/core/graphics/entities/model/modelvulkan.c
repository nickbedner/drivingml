#include "mana/core/graphics/entities/model/modelvulkan.h"

void model_vulkan_clone_init(struct ModelCommon* model_common, struct APICommon* api_common) {
  vulkan_graphics_utils_setup_vertex_buffer(&(api_common->vulkan_api), model_common->model_mesh->mesh_common.vertices, &(model_common->model_vulkan.vertex_buffer), &(model_common->model_vulkan.vertex_buffer_memory));
  vulkan_graphics_utils_setup_index_buffer(&(api_common->vulkan_api), model_common->model_mesh->mesh_common.indices, &(model_common->model_vulkan.index_buffer), &(model_common->model_vulkan.index_buffer_memory));
  vulkan_graphics_utils_setup_uniform_buffer(&(api_common->vulkan_api), sizeof(struct ModelUniformBufferObject), &(model_common->model_vulkan.uniform_buffer), &(model_common->model_vulkan.uniform_buffers_memory));
  vulkan_graphics_utils_setup_uniform_buffer(&(api_common->vulkan_api), sizeof(struct LightingUniformBufferObject), &(model_common->model_vulkan.lighting_uniform_buffer), &(model_common->model_vulkan.lighting_uniform_buffers_memory));
  if (model_common->animated)
    vulkan_graphics_utils_setup_uniform_buffer(&(api_common->vulkan_api), sizeof(struct ModelAnimationUniformBufferObject), &(model_common->model_vulkan.uniform_animation_buffer), &(model_common->model_vulkan.uniform_animation_buffers_memory));

  VkWriteDescriptorSet dcs[8];
  memset(dcs, 0, sizeof(dcs));
  VkDescriptorBufferInfo model_buffer_info = vulkan_graphics_utils_setup_descriptor_buffer_info(sizeof(struct ModelUniformBufferObject), &(model_common->model_vulkan.uniform_buffer));
  VkDescriptorBufferInfo lighting_buffer_info = vulkan_graphics_utils_setup_descriptor_buffer_info(sizeof(struct LightingUniformBufferObject), &(model_common->model_vulkan.lighting_uniform_buffer));
  vulkan_graphics_utils_setup_descriptor_buffer(dcs, 0, model_common->model_vulkan.descriptor_set, &model_buffer_info);
  vulkan_graphics_utils_setup_descriptor_buffer(dcs, 1, model_common->model_vulkan.descriptor_set, &lighting_buffer_info);
  vulkan_graphics_utils_setup_descriptor_image(dcs, 2, model_common->model_vulkan.descriptor_set, (VkDescriptorImageInfo[]){vulkan_graphics_utils_setup_descriptor_image_info(&(model_common->model_diffuse_texture->texture_common.texture_vulkan.texture_image_view), &(model_common->model_diffuse_texture->texture_common.texture_vulkan.texture_sampler))});
  vulkan_graphics_utils_setup_descriptor_image(dcs, 3, model_common->model_vulkan.descriptor_set, (VkDescriptorImageInfo[]){vulkan_graphics_utils_setup_descriptor_image_info(&(model_common->model_normal_texture->texture_common.texture_vulkan.texture_image_view), &(model_common->model_normal_texture->texture_common.texture_vulkan.texture_sampler))});
  vulkan_graphics_utils_setup_descriptor_image(dcs, 4, model_common->model_vulkan.descriptor_set, (VkDescriptorImageInfo[]){vulkan_graphics_utils_setup_descriptor_image_info(&(model_common->model_metallic_texture->texture_common.texture_vulkan.texture_image_view), &(model_common->model_metallic_texture->texture_common.texture_vulkan.texture_sampler))});
  vulkan_graphics_utils_setup_descriptor_image(dcs, 5, model_common->model_vulkan.descriptor_set, (VkDescriptorImageInfo[]){vulkan_graphics_utils_setup_descriptor_image_info(&(model_common->model_roughness_texture->texture_common.texture_vulkan.texture_image_view), &(model_common->model_roughness_texture->texture_common.texture_vulkan.texture_sampler))});
  vulkan_graphics_utils_setup_descriptor_image(dcs, 6, model_common->model_vulkan.descriptor_set, (VkDescriptorImageInfo[]){vulkan_graphics_utils_setup_descriptor_image_info(&(model_common->model_ao_texture->texture_common.texture_vulkan.texture_image_view), &(model_common->model_ao_texture->texture_common.texture_vulkan.texture_sampler))});

  if (model_common->animated) {
    vulkan_graphics_utils_setup_descriptor_buffer(dcs, 7, model_common->model_vulkan.descriptor_set, (VkDescriptorBufferInfo[]){vulkan_graphics_utils_setup_descriptor_buffer_info(sizeof(struct ModelAnimationUniformBufferObject), &(model_common->model_vulkan.uniform_animation_buffer))});
    vkUpdateDescriptorSets(api_common->vulkan_api.device, 8, dcs, 0, NULL);
  } else
    vkUpdateDescriptorSets(api_common->vulkan_api.device, 7, dcs, 0, NULL);
}

void model_vulkan_clone_delete(struct ModelCommon* model_common, struct APICommon* api_common) {
  vkDestroyBuffer(api_common->vulkan_api.device, model_common->model_vulkan.index_buffer, NULL);
  vkFreeMemory(api_common->vulkan_api.device, model_common->model_vulkan.index_buffer_memory, NULL);
  vkDestroyBuffer(api_common->vulkan_api.device, model_common->model_vulkan.vertex_buffer, NULL);
  vkFreeMemory(api_common->vulkan_api.device, model_common->model_vulkan.vertex_buffer_memory, NULL);
  vkDestroyBuffer(api_common->vulkan_api.device, model_common->model_vulkan.uniform_buffer, NULL);
  vkFreeMemory(api_common->vulkan_api.device, model_common->model_vulkan.uniform_buffers_memory, NULL);
  vkDestroyBuffer(api_common->vulkan_api.device, model_common->model_vulkan.lighting_uniform_buffer, NULL);
  vkFreeMemory(api_common->vulkan_api.device, model_common->model_vulkan.lighting_uniform_buffers_memory, NULL);
  if (model_common->animated) {
    vkDestroyBuffer(api_common->vulkan_api.device, model_common->model_vulkan.uniform_animation_buffer, NULL);
    vkFreeMemory(api_common->vulkan_api.device, model_common->model_vulkan.uniform_animation_buffers_memory, NULL);
  }
}

void model_vulkan_render(struct ModelCommon* model_common, struct GBuffer* gbuffer, r64 delta_time) {
  //(void)delta_time;
  VkCommandBuffer cmd = gbuffer->gbuffer_common.gbuffer_vulkan.command_buffer;
  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, model_common->shader_handle->shader_common.shader_vulkan.graphics_pipeline);
  vkCmdSetViewport(cmd, 0, 1, &(model_common->shader_handle->shader_common.shader_vulkan.viewport));
  vkCmdSetScissor(cmd, 0, 1, &(model_common->shader_handle->shader_common.shader_vulkan.scissor));

  VkBuffer vertex_buffers[] = {model_common->model_vulkan.vertex_buffer};
  VkDeviceSize offsets[] = {0};

  vkCmdBindVertexBuffers(cmd, 0, 1, vertex_buffers, offsets);
  vkCmdBindIndexBuffer(cmd, model_common->model_vulkan.index_buffer, 0, VK_INDEX_TYPE_UINT32);
  vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, model_common->shader_handle->shader_common.shader_vulkan.pipeline_layout, 0, 1, model_common->model_vulkan.descriptor_set, 0, NULL);
  vkCmdDrawIndexed(cmd, (u32)model_common->model_mesh->mesh_common.indices->size, 1, 0, 0, 0);
}

void model_vulkan_update_uniforms(struct ModelCommon* model_common, struct APICommon* api_common, struct GBuffer* gbuffer, vec3d position, vec4 light_pos, vec4 diffuse_color, vec4 ambient_color, vec4 specular_light) {
  struct LightingUniformBufferObject light_ubo = {0};
  light_ubo.direction = light_pos;
  light_ubo.ambient_color = ambient_color;
  light_ubo.diffuse_color = diffuse_color;
  light_ubo.specular_color = specular_light;

  struct ModelUniformBufferObject ubom = {0};
  ubom.proj = gbuffer->gbuffer_common.projection_matrix;
  ubom.view = gbuffer->gbuffer_common.view_matrix;
  ubom.model = mat4_translate(MAT4_IDENTITY, model_common->position);
  ubom.model = mat4_mul(ubom.model, quaternion_to_mat4(quaternion_normalise(model_common->rotation)));
  ubom.model = mat4_scale(ubom.model, model_common->scale);
  ubom.camera_pos = vec3d_to_vec3(position);

  void* data = NULL;
  vkMapMemory(api_common->vulkan_api.device, model_common->model_vulkan.uniform_buffers_memory, 0, sizeof(struct ModelUniformBufferObject), 0, &data);
  memcpy(data, &ubom, sizeof(struct ModelUniformBufferObject));
  vkUnmapMemory(api_common->vulkan_api.device, model_common->model_vulkan.uniform_buffers_memory);

  if (model_common->animated) {
    struct ModelAnimationUniformBufferObject uboa = {{{0}}};
    model_get_joint_transforms(model_common->root_joint, uboa.joint_transforms);

    void* animation_data = NULL;
    vkMapMemory(api_common->vulkan_api.device, model_common->model_vulkan.uniform_animation_buffers_memory, 0, sizeof(struct ModelAnimationUniformBufferObject), 0, &animation_data);
    memcpy(animation_data, &uboa, sizeof(struct ModelAnimationUniformBufferObject));
    vkUnmapMemory(api_common->vulkan_api.device, model_common->model_vulkan.uniform_animation_buffers_memory);
  }

  void* lighting_data = NULL;
  vkMapMemory(api_common->vulkan_api.device, model_common->model_vulkan.lighting_uniform_buffers_memory, 0, sizeof(struct LightingUniformBufferObject), 0, &lighting_data);
  memcpy(lighting_data, &light_ubo, sizeof(struct LightingUniformBufferObject));
  vkUnmapMemory(api_common->vulkan_api.device, model_common->model_vulkan.lighting_uniform_buffers_memory);
}
