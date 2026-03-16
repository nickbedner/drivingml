#include "mana/graphics/render/postprocess/postprocessvulkan.h"

static inline uint_fast8_t post_process_vulkan_init_common(struct PostProcessCommon* post_process_common, struct APICommon* api_common, struct SwapChainCommon* swap_chain_common) {
  enum VkFormat image_format = VK_FORMAT_R8G8B8A8_UNORM;  // VK_FORMAT_R16G16B16A16_SFLOAT;
  struct VkAttachmentDescription color_attachment = {0};
  vulkan_graphics_utils_create_color_attachment(swap_chain_common->swap_chain_vulkan.swap_chain_image_format, &color_attachment);
  color_attachment.format = image_format;
  color_attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

  VkAttachmentReference color_attachment_ref = {0};
  color_attachment_ref.attachment = 0;
  color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  // color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;

  struct VkAttachmentReference color_attachment_reference = color_attachment_ref;
  VkSubpassDescription subpass = {0};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_attachment_reference;
  subpass.pDepthStencilAttachment = VK_NULL_HANDLE;

  VkSubpassDependency dependencies[POST_PROCESS_PING_PONG];
  dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
  dependencies[0].dstSubpass = 0;
  dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
  dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  dependencies[1].srcSubpass = 0;
  dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
  dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
  dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  VkAttachmentDescription attachments_render_pass = color_attachment;
  VkRenderPassCreateInfo render_pass_info = {0};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  render_pass_info.pAttachments = &attachments_render_pass;
  render_pass_info.attachmentCount = 1;
  render_pass_info.subpassCount = 1;
  render_pass_info.pSubpasses = &subpass;
  render_pass_info.dependencyCount = POST_PROCESS_PING_PONG;
  render_pass_info.pDependencies = dependencies;

  if (vkCreateRenderPass(api_common->vulkan_api.device, &render_pass_info, NULL, &(post_process_common->post_process_vulkan.render_pass)) != VK_SUCCESS)
    return 0;

  for (uint_fast8_t ping_pong_target = 0; ping_pong_target < POST_PROCESS_PING_PONG; ping_pong_target++) {
    // vulkan_graphics_utils_create_image(api_common->vulkan_api.device, api_common->vulkan_api.physical_device, swap_chain_common->swap_chain_extent.width, swap_chain_common->swap_chain_extent.height, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &(post_process_common->post_process_vulkan.color_images[ping_pong_target]), &(post_process_common->post_process_vulkan.color_image_memories[ping_pong_target]));
    //  vulkan_graphics_utils_create_image_view(api_common->vulkan_api.device, post_process_common->post_process_vulkan.color_images[ping_pong_target], VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT, 1, &(post_process_common->post_process_vulkan.color_image_views[ping_pong_target]));
    vulkan_graphics_utils_create_image(api_common->vulkan_api.device, api_common->vulkan_api.physical_device, swap_chain_common->swap_chain_extent.width, swap_chain_common->swap_chain_extent.height, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &(post_process_common->post_process_vulkan.color_images[ping_pong_target]), &(post_process_common->post_process_vulkan.color_image_memories[ping_pong_target]));
    vulkan_graphics_utils_create_image_view(api_common->vulkan_api.device, post_process_common->post_process_vulkan.color_images[ping_pong_target], VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, &(post_process_common->post_process_vulkan.color_image_views[ping_pong_target]));

    VkImageView attachments_framebuffer = post_process_common->post_process_vulkan.color_image_views[ping_pong_target];
    VkFramebufferCreateInfo framebuffer_info = {0};
    framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_info.renderPass = post_process_common->post_process_vulkan.render_pass;
    framebuffer_info.attachmentCount = 1;
    framebuffer_info.pAttachments = &attachments_framebuffer;
    framebuffer_info.width = swap_chain_common->swap_chain_extent.width;
    framebuffer_info.height = swap_chain_common->swap_chain_extent.height;
    framebuffer_info.layers = 1;

    if (vkCreateFramebuffer(api_common->vulkan_api.device, &framebuffer_info, NULL, &(post_process_common->post_process_vulkan.framebuffer[ping_pong_target])) != VK_SUCCESS) {
      log_message(LOG_SEVERITY_ERROR, "Failed to create framebuffer!\n");
      return VULKAN_RENDERER_CREATE_FRAME_BUFFER_ERROR;
    }
  }

  return 0;
}

// Note: I'm thinking only need to use supersample once when writing from color texture to post process. When post process is ping ponging it should be regular resolution
static inline void post_process_vulkan_update_uniform_buffer(struct PostProcessCommon* post_process_common, struct APICommon* api_common, struct SwapChainCommon* swap_chain_common) {
  struct ResolveUniformBufferObject ubos = {.screen_size = (vec2){.x = (float)swap_chain_common->swap_chain_extent.width, .y = (float)swap_chain_common->swap_chain_extent.height}};
  void* data;
  vkMapMemory(api_common->vulkan_api.device, post_process_common->post_process_vulkan.uniform_buffers_memory, 0, sizeof(struct ResolveUniformBufferObject), 0, &data);
  memcpy(data, &ubos, sizeof(struct ResolveUniformBufferObject));
  vkUnmapMemory(api_common->vulkan_api.device, post_process_common->post_process_vulkan.uniform_buffers_memory);
}

uint_fast8_t post_process_vulkan_init(struct PostProcessCommon* post_process_common, struct APICommon* api_common, struct SwapChainCommon* swap_chain_common) {
  if (post_process_vulkan_init_common(post_process_common, api_common, swap_chain_common) != 0)
    return 1;

  for (uint_fast8_t ping_pong_target = 0; ping_pong_target < POST_PROCESS_PING_PONG; ping_pong_target++) {
    VkSemaphoreCreateInfo semaphore_info = {0};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    vkCreateSemaphore(api_common->vulkan_api.device, &semaphore_info, NULL, &(post_process_common->post_process_vulkan.semaphore[ping_pong_target]));
  }

  vulkan_graphics_utils_create_sampler(api_common->vulkan_api.device, &(post_process_common->post_process_vulkan.texture_sampler), (struct SamplerSettings){.mip_levels = 1, .min_filter = VK_FILTER_LINEAR, .mag_filter = VK_FILTER_LINEAR, .mipmap_mode = VK_SAMPLER_MIPMAP_MODE_NEAREST, .address_mode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, .anisotropy_enable = VK_FALSE, .max_anisotropy = 1.0f});
  // Post process command buffer
  VkCommandBufferAllocateInfo alloc_info_post_process = {0};
  alloc_info_post_process.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info_post_process.commandPool = api_common->vulkan_api.command_pool;
  alloc_info_post_process.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info_post_process.commandBufferCount = POST_PROCESS_PING_PONG;

  if (vkAllocateCommandBuffers(api_common->vulkan_api.device, &alloc_info_post_process, (post_process_common->post_process_vulkan.command_buffer)) != VK_SUCCESS) {
    log_message(LOG_SEVERITY_ERROR, "Failed to begin recording command buffer!\n");
    return VULKAN_RENDERER_CREATE_COMMAND_BUFFER_ERROR;
  }

  vulkan_graphics_utils_setup_uniform_buffer(&api_common->vulkan_api, sizeof(struct ResolveUniformBufferObject), &(post_process_common->post_process_vulkan.uniform_buffer), &(post_process_common->post_process_vulkan.uniform_buffers_memory));
  post_process_vulkan_update_uniform_buffer(post_process_common, api_common, swap_chain_common);

  post_process_common->resolve_shader->extra_data = post_process_common->post_process_vulkan.render_pass;

  return 0;
}

static inline void post_process_delete_common(struct PostProcessCommon* post_process_common, struct APICommon* api_common) {
  vkDestroyRenderPass(api_common->vulkan_api.device, post_process_common->post_process_vulkan.render_pass, NULL);

  for (uint_fast8_t ping_pong_target = 0; ping_pong_target <= 1; ping_pong_target++) {
    vkDestroyFramebuffer(api_common->vulkan_api.device, post_process_common->post_process_vulkan.framebuffer[ping_pong_target], NULL);
    vkDestroyImageView(api_common->vulkan_api.device, post_process_common->post_process_vulkan.color_image_views[ping_pong_target], NULL);
    vkDestroyImage(api_common->vulkan_api.device, post_process_common->post_process_vulkan.color_images[ping_pong_target], NULL);
    vkFreeMemory(api_common->vulkan_api.device, post_process_common->post_process_vulkan.color_image_memories[ping_pong_target], NULL);
  }
}

void post_process_vulkan_delete(struct PostProcessCommon* post_process_common, struct APICommon* api_common) {
  post_process_delete_common(post_process_common, api_common);

  vkDestroySampler(api_common->vulkan_api.device, post_process_common->post_process_vulkan.texture_sampler, NULL);

  vkDestroyBuffer(api_common->vulkan_api.device, post_process_common->post_process_vulkan.uniform_buffer, NULL);
  vkFreeMemory(api_common->vulkan_api.device, post_process_common->post_process_vulkan.uniform_buffers_memory, NULL);

  vkDestroyBuffer(api_common->vulkan_api.device, post_process_common->post_process_vulkan.vertex_buffer, NULL);
  vkFreeMemory(api_common->vulkan_api.device, post_process_common->post_process_vulkan.vertex_buffer_memory, NULL);
  vkDestroyBuffer(api_common->vulkan_api.device, post_process_common->post_process_vulkan.index_buffer, NULL);
  vkFreeMemory(api_common->vulkan_api.device, post_process_common->post_process_vulkan.index_buffer_memory, NULL);

  for (uint_fast8_t ping_pong_target = 0; ping_pong_target <= 1; ping_pong_target++)
    vkDestroySemaphore(api_common->vulkan_api.device, post_process_common->post_process_vulkan.semaphore[ping_pong_target], NULL);
}

uint_fast8_t post_process_vulkan_resize(struct PostProcessCommon* post_process_common, struct APICommon* api_common, struct SwapChainCommon* swap_chain_common) {
  post_process_delete_common(post_process_common, api_common);
  post_process_vulkan_init_common(post_process_common, api_common, swap_chain_common);

  post_process_vulkan_update_uniform_buffer(post_process_common, api_common, swap_chain_common);

  return 0;
}

uint_fast8_t post_process_vulkan_resolve_init(struct PostProcessCommon* post_process_common, struct APICommon* api_common, struct GBuffer* gbuffer, struct SwapChainCommon* swap_chain_common) {
  for (uint_fast8_t shader_num = 0; shader_num < 4; shader_num++) {
    vulkan_graphics_utils_create_descriptors(&(api_common->vulkan_api), &(post_process_common->post_process_vulkan.descriptor_set[shader_num]), &(post_process_common->resolve_shader->shader[shader_num].shader_common.shader_vulkan.descriptor_set_layout), &(post_process_common->resolve_shader->shader[shader_num].shader_common.shader_vulkan.descriptor_pool), post_process_common->resolve_shader->shader[shader_num].shader_common.shader_settings.descriptors);

    VkWriteDescriptorSet dcs[2] = {0};
    vulkan_graphics_utils_setup_descriptor_buffer(dcs, 0, &(post_process_common->post_process_vulkan.descriptor_set[shader_num]), (VkDescriptorBufferInfo[]){vulkan_graphics_utils_setup_descriptor_buffer_info(sizeof(struct ResolveUniformBufferObject), &(post_process_common->post_process_vulkan.uniform_buffer))});
    vulkan_graphics_utils_setup_descriptor_image(dcs, 1, &(post_process_common->post_process_vulkan.descriptor_set[shader_num]), (VkDescriptorImageInfo[]){vulkan_graphics_utils_setup_descriptor_image_info(&(gbuffer->gbuffer_common.gbuffer_vulkan.color_image_view), &(gbuffer->gbuffer_common.gbuffer_vulkan.texture_sampler))});
    vkUpdateDescriptorSets(api_common->vulkan_api.device, 2, dcs, 0, NULL);
  }

  vulkan_graphics_utils_setup_vertex_buffer(&api_common->vulkan_api, post_process_common->blit_fullscreen_triangle.mesh_common.vertices, &(post_process_common->post_process_vulkan.vertex_buffer), &(post_process_common->post_process_vulkan.vertex_buffer_memory));
  vulkan_graphics_utils_setup_index_buffer(&api_common->vulkan_api, post_process_common->blit_fullscreen_triangle.mesh_common.indices, &(post_process_common->post_process_vulkan.index_buffer), &(post_process_common->post_process_vulkan.index_buffer_memory));

  return 0;
}

uint_fast8_t post_process_vulkan_resolve_update(struct PostProcessCommon* post_process_common, struct APICommon* api_common, struct GBuffer* gbuffer, struct SwapChainCommon* swap_chain_common) {
  for (uint_fast8_t shader_num = 0; shader_num < 4; shader_num++) {
    VkWriteDescriptorSet dcs[2] = {0};
    vulkan_graphics_utils_setup_descriptor_buffer(dcs, 0, &(post_process_common->post_process_vulkan.descriptor_set[shader_num]), (VkDescriptorBufferInfo[]){vulkan_graphics_utils_setup_descriptor_buffer_info(sizeof(struct ResolveUniformBufferObject), &(post_process_common->post_process_vulkan.uniform_buffer))});
    vulkan_graphics_utils_setup_descriptor_image(dcs, 1, &(post_process_common->post_process_vulkan.descriptor_set[shader_num]), (VkDescriptorImageInfo[]){vulkan_graphics_utils_setup_descriptor_image_info(&(gbuffer->gbuffer_common.gbuffer_vulkan.color_image_view), &(gbuffer->gbuffer_common.gbuffer_vulkan.texture_sampler))});
    vkUpdateDescriptorSets(api_common->vulkan_api.device, 2, dcs, 0, NULL);
  }
  return 0;
}

uint_fast8_t post_process_vulkan_resolve_render(struct PostProcessCommon* post_process_common, struct APICommon* api_common, struct GBuffer* gbuffer, struct SwapChainCommon* swap_chain_common) {
  // Custom for intial blitting to post process image
  VkCommandBufferBeginInfo begin_info = {0};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

  if (vkBeginCommandBuffer(post_process_common->post_process_vulkan.command_buffer[post_process_common->ping_pong], &begin_info) != VK_SUCCESS) {
    log_message(LOG_SEVERITY_ERROR, "Failed to begin recording command buffer!\n");
    return VULKAN_RENDERER_CREATE_COMMAND_BUFFER_ERROR;
  }

  // VkClearValue clear_color = {.color = {0.0f, 1.0f, 0.0f, 1.0f}}; // RGBA: Red
  VkRenderPassBeginInfo render_pass_info = {0};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  render_pass_info.renderPass = post_process_common->post_process_vulkan.render_pass;
  render_pass_info.framebuffer = post_process_common->post_process_vulkan.framebuffer[post_process_common->ping_pong];
  render_pass_info.renderArea.offset.x = 0;
  render_pass_info.renderArea.offset.y = 0;
  render_pass_info.renderArea.extent = (VkExtent2D){.width = swap_chain_common->swap_chain_extent.width, .height = swap_chain_common->swap_chain_extent.height};
  // render_pass_info.clearValueCount = 1;
  // render_pass_info.pClearValues = &clear_color;
  render_pass_info.clearValueCount = 0;
  render_pass_info.pClearValues = NULL;

  vkCmdBeginRenderPass(post_process_common->post_process_vulkan.command_buffer[post_process_common->ping_pong], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
  vkCmdBindPipeline(post_process_common->post_process_vulkan.command_buffer[post_process_common->ping_pong], VK_PIPELINE_BIND_POINT_GRAPHICS, post_process_common->resolve_shader->shader[swap_chain_common->supersample_scale - 1].shader_common.shader_vulkan.graphics_pipeline);
  VkBuffer vertex_buffers[] = {post_process_common->post_process_vulkan.vertex_buffer};
  VkDeviceSize offsets[] = {0};
  vkCmdSetViewport(post_process_common->post_process_vulkan.command_buffer[post_process_common->ping_pong], 0, 1, &(post_process_common->resolve_shader->shader[swap_chain_common->supersample_scale - 1].shader_common.shader_vulkan.viewport));
  vkCmdSetScissor(post_process_common->post_process_vulkan.command_buffer[post_process_common->ping_pong], 0, 1, &(post_process_common->resolve_shader->shader[swap_chain_common->supersample_scale - 1].shader_common.shader_vulkan.scissor));
  vkCmdBindVertexBuffers(post_process_common->post_process_vulkan.command_buffer[post_process_common->ping_pong], 0, 1, vertex_buffers, offsets);
  vkCmdBindIndexBuffer(post_process_common->post_process_vulkan.command_buffer[post_process_common->ping_pong], post_process_common->post_process_vulkan.index_buffer, 0, VK_INDEX_TYPE_UINT32);
  vkCmdBindDescriptorSets(post_process_common->post_process_vulkan.command_buffer[post_process_common->ping_pong], VK_PIPELINE_BIND_POINT_GRAPHICS, post_process_common->resolve_shader->shader[swap_chain_common->supersample_scale - 1].shader_common.shader_vulkan.pipeline_layout, 0, 1, post_process_common->post_process_vulkan.descriptor_set, 0, NULL);
  vkCmdDrawIndexed(post_process_common->post_process_vulkan.command_buffer[post_process_common->ping_pong], (uint32_t)post_process_common->blit_fullscreen_triangle.mesh_common.indices->size, 1, 0, 0, 0);
  vkCmdEndRenderPass(post_process_common->post_process_vulkan.command_buffer[post_process_common->ping_pong]);

  if (vkEndCommandBuffer(post_process_common->post_process_vulkan.command_buffer[post_process_common->ping_pong]) != VK_SUCCESS) {
    log_message(LOG_SEVERITY_ERROR, "Failed to begin recording command buffer!\n");
    return VULKAN_RENDERER_CREATE_COMMAND_BUFFER_ERROR;
  }

  // Send to GPU for offscreen rendering then wait until finished
  VkSubmitInfo post_process_submit_info = {0};
  post_process_submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  VkSemaphore wait_semaphore = gbuffer->gbuffer_common.gbuffer_vulkan.semaphore;
  VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  post_process_submit_info.waitSemaphoreCount = 1;
  post_process_submit_info.pWaitSemaphores = &wait_semaphore;
  post_process_submit_info.pWaitDstStageMask = &wait_stage;

  post_process_submit_info.commandBufferCount = 1;
  post_process_submit_info.pCommandBuffers = &(post_process_common->post_process_vulkan.command_buffer[post_process_common->ping_pong]);
  VkSemaphore post_process_signal_semaphore = post_process_common->post_process_vulkan.semaphore[post_process_common->ping_pong];
  post_process_submit_info.signalSemaphoreCount = 1;
  post_process_submit_info.pSignalSemaphores = &post_process_signal_semaphore;

  VkResult result = vkQueueSubmit(api_common->vulkan_api.graphics_queue, 1, &post_process_submit_info, VK_NULL_HANDLE);

  if (result != VK_SUCCESS) {
    fprintf(stderr, "Error to submit GBuffer!\n");
    return 1;
  }

  return 0;
}
