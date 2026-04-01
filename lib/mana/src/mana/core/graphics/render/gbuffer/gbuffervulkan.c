#include "mana/core/graphics/render/gbuffer/gbuffervulkan.h"

internal inline u8 gbuffer_vulkan_init_common(struct GBufferCommon* gbuffer_common, struct APICommon* api_common, struct SwapChainCommon* swap_chain_common, const uint_fast32_t msaa_samples) {
  VkFormat image_format = VK_FORMAT_R8G8B8A8_UNORM;  // VK_FORMAT_R16G16B16A16_SFLOAT;
  VkImageUsageFlags image_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  VkFormat depth_format = vulkan_graphics_utils_find_depth_format(api_common->vulkan_api.physical_device);

  u32 gbuffer_width = swap_chain_common->swap_chain_extent.width * swap_chain_common->supersample_scale;
  u32 gbuffer_height = swap_chain_common->swap_chain_extent.height * swap_chain_common->supersample_scale;

  // Resolve or standard if not multisampling
  vulkan_graphics_utils_create_image(api_common->vulkan_api.device, api_common->vulkan_api.physical_device, gbuffer_width, gbuffer_height, 1, 1, VK_SAMPLE_COUNT_1_BIT, image_format, VK_IMAGE_TILING_OPTIMAL, image_usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &(gbuffer_common->gbuffer_vulkan.color_image), &(gbuffer_common->gbuffer_vulkan.color_image_memory));
  vulkan_graphics_utils_create_image_view(api_common->vulkan_api.device, gbuffer_common->gbuffer_vulkan.color_image, image_format, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, FALSE, &(gbuffer_common->gbuffer_vulkan.color_image_view));

  vulkan_graphics_utils_create_image(api_common->vulkan_api.device, api_common->vulkan_api.physical_device, gbuffer_width, gbuffer_height, 1, 1, VK_SAMPLE_COUNT_1_BIT, image_format, VK_IMAGE_TILING_OPTIMAL, image_usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &(gbuffer_common->gbuffer_vulkan.normal_image), &(gbuffer_common->gbuffer_vulkan.normal_image_memory));
  vulkan_graphics_utils_create_image_view(api_common->vulkan_api.device, gbuffer_common->gbuffer_vulkan.normal_image, image_format, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, FALSE, &(gbuffer_common->gbuffer_vulkan.normal_image_view));

  vulkan_graphics_utils_create_image(api_common->vulkan_api.device, api_common->vulkan_api.physical_device, gbuffer_width, gbuffer_height, 1, 1, (VkSampleCountFlagBits)msaa_samples, depth_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &(gbuffer_common->gbuffer_vulkan.depth_image), &(gbuffer_common->gbuffer_vulkan.depth_image_memory));
  vulkan_graphics_utils_create_image_view(api_common->vulkan_api.device, gbuffer_common->gbuffer_vulkan.depth_image, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT, 1, 1, FALSE, &(gbuffer_common->gbuffer_vulkan.depth_image_view));
  VkAttachmentDescription color_attachment = {0};
  vulkan_graphics_utils_create_color_attachment(swap_chain_common->swap_chain_vulkan.swap_chain_image_format, &color_attachment);
  color_attachment.format = image_format;
  color_attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  // color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;

  VkAttachmentDescription normal_attachment = {0};
  vulkan_graphics_utils_create_color_attachment(swap_chain_common->swap_chain_vulkan.swap_chain_image_format, &normal_attachment);
  normal_attachment.format = image_format;
  normal_attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

  VkAttachmentDescription depth_attachment = {0};
  vulkan_graphics_utils_create_depth_attachment(api_common->vulkan_api.physical_device, &depth_attachment);
  depth_attachment.samples = (VkSampleCountFlagBits)msaa_samples;

  VkAttachmentReference color_attachment_ref = {0};
  color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentReference normal_attachment_ref = {0};
  normal_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depth_attachment_ref = {0};
  depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  if (msaa_samples != 1) {
    color_attachment_ref.attachment = 3;
    normal_attachment_ref.attachment = 4;
    depth_attachment_ref.attachment = 2;

    // Multisample
    vulkan_graphics_utils_create_image(api_common->vulkan_api.device, api_common->vulkan_api.physical_device, gbuffer_width, gbuffer_height, 1, 1, (VkSampleCountFlagBits)msaa_samples, image_format, VK_IMAGE_TILING_OPTIMAL, image_usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &(gbuffer_common->gbuffer_vulkan.multisample_color_image), &(gbuffer_common->gbuffer_vulkan.multisample_color_image_memory));
    vulkan_graphics_utils_create_image_view(api_common->vulkan_api.device, gbuffer_common->gbuffer_vulkan.multisample_color_image, image_format, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, FALSE, &(gbuffer_common->gbuffer_vulkan.multisample_color_image_view));

    vulkan_graphics_utils_create_image(api_common->vulkan_api.device, api_common->vulkan_api.physical_device, gbuffer_width, gbuffer_height, 1, 1, (VkSampleCountFlagBits)msaa_samples, image_format, VK_IMAGE_TILING_OPTIMAL, image_usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &(gbuffer_common->gbuffer_vulkan.multisample_normal_image), &(gbuffer_common->gbuffer_vulkan.multisample_normal_image_memory));
    vulkan_graphics_utils_create_image_view(api_common->vulkan_api.device, gbuffer_common->gbuffer_vulkan.multisample_normal_image, image_format, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, FALSE, &(gbuffer_common->gbuffer_vulkan.multisample_normal_image_view));

    VkAttachmentDescription multisample_color_attachment = {0};
    vulkan_graphics_utils_create_color_attachment(swap_chain_common->swap_chain_vulkan.swap_chain_image_format, &multisample_color_attachment);
    multisample_color_attachment.format = image_format;
    multisample_color_attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    multisample_color_attachment.samples = (VkSampleCountFlagBits)msaa_samples;
    // multisample_color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;

    VkAttachmentDescription multisample_normal_attachment = {0};
    vulkan_graphics_utils_create_color_attachment(swap_chain_common->swap_chain_vulkan.swap_chain_image_format, &multisample_normal_attachment);
    multisample_normal_attachment.format = image_format;
    multisample_normal_attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    multisample_normal_attachment.samples = (VkSampleCountFlagBits)msaa_samples;

    VkAttachmentReference multisample_color_attachment_ref = {0};
    multisample_color_attachment_ref.attachment = 0;
    multisample_color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference multisample_normal_attachment_ref = {0};
    multisample_normal_attachment_ref.attachment = 1;
    multisample_normal_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    ////////////////////////////////////////////////////////////////////////////////////

    VkAttachmentReference multisample_attachment_references[GBUFFER_COLOR_ATTACHMENTS] = {multisample_color_attachment_ref, multisample_normal_attachment_ref};
    VkAttachmentReference color_attachment_references[GBUFFER_COLOR_ATTACHMENTS] = {color_attachment_ref, normal_attachment_ref};
    VkSubpassDescription subpass = {0};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = GBUFFER_COLOR_ATTACHMENTS;
    subpass.pColorAttachments = multisample_attachment_references;
    subpass.pDepthStencilAttachment = &depth_attachment_ref;
    subpass.pResolveAttachments = color_attachment_references;

    VkSubpassDependency dependencies[GBUFFER_TOTAL_DEPENDENCIES];
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

    VkRenderPassCreateInfo render_pass_info;
    memset(&render_pass_info, 0, sizeof(render_pass_info));
    VkAttachmentDescription attachments_render_pass[MULTISAMPLE_GBUFFER_TOTAL_ATTACHMENTS] = {multisample_color_attachment, multisample_normal_attachment, depth_attachment, color_attachment, normal_attachment};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.pAttachments = attachments_render_pass;
    render_pass_info.attachmentCount = MULTISAMPLE_GBUFFER_TOTAL_ATTACHMENTS;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = GBUFFER_TOTAL_DEPENDENCIES;
    render_pass_info.pDependencies = dependencies;

    if (vkCreateRenderPass(api_common->vulkan_api.device, &render_pass_info, NULL, &(gbuffer_common->gbuffer_vulkan.render_pass)) != VK_SUCCESS)
      return 0;

    VkFramebufferCreateInfo framebuffer_info;
    memset(&framebuffer_info, 0, sizeof(framebuffer_info));
    VkImageView attachments_framebuffer[] = {gbuffer_common->gbuffer_vulkan.multisample_color_image_view, gbuffer_common->gbuffer_vulkan.multisample_normal_image_view, gbuffer_common->gbuffer_vulkan.depth_image_view, gbuffer_common->gbuffer_vulkan.color_image_view, gbuffer_common->gbuffer_vulkan.normal_image_view};
    framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_info.renderPass = gbuffer_common->gbuffer_vulkan.render_pass;
    framebuffer_info.attachmentCount = MULTISAMPLE_GBUFFER_TOTAL_ATTACHMENTS;
    framebuffer_info.pAttachments = attachments_framebuffer;
    framebuffer_info.width = gbuffer_width;
    framebuffer_info.height = gbuffer_height;
    framebuffer_info.layers = 1;

    if (vkCreateFramebuffer(api_common->vulkan_api.device, &framebuffer_info, NULL, &(gbuffer_common->gbuffer_vulkan.framebuffer)) != VK_SUCCESS) {
      log_message(LOG_SEVERITY_ERROR, "Failed to create framebuffer!\n");
      return VULKAN_RENDERER_CREATE_FRAME_BUFFER_ERROR;
    }
  } else {
    color_attachment_ref.attachment = 0;
    normal_attachment_ref.attachment = 1;
    depth_attachment_ref.attachment = 2;

    struct VkAttachmentReference color_attachment_references[GBUFFER_COLOR_ATTACHMENTS] = {color_attachment_ref, normal_attachment_ref};
    VkSubpassDescription subpass = {0};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = GBUFFER_COLOR_ATTACHMENTS;
    subpass.pColorAttachments = color_attachment_references;
    subpass.pDepthStencilAttachment = &depth_attachment_ref;

    VkSubpassDependency dependencies[GBUFFER_TOTAL_DEPENDENCIES];
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

    VkAttachmentDescription attachments_render_pass[GBUFFER_TOTAL_ATTACHMENTS] = {color_attachment, normal_attachment, depth_attachment};
    VkRenderPassCreateInfo render_pass_info;
    memset(&render_pass_info, 0, sizeof(render_pass_info));
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.pAttachments = attachments_render_pass;
    render_pass_info.attachmentCount = GBUFFER_TOTAL_ATTACHMENTS;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = GBUFFER_TOTAL_DEPENDENCIES;
    render_pass_info.pDependencies = dependencies;

    if (vkCreateRenderPass(api_common->vulkan_api.device, &render_pass_info, NULL, &(gbuffer_common->gbuffer_vulkan.render_pass)) != VK_SUCCESS)
      return 0;

    VkFramebufferCreateInfo framebuffer_info;
    memset(&framebuffer_info, 0, sizeof(framebuffer_info));
    VkImageView attachments_framebuffer[] = {gbuffer_common->gbuffer_vulkan.color_image_view, gbuffer_common->gbuffer_vulkan.normal_image_view, gbuffer_common->gbuffer_vulkan.depth_image_view};
    framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_info.renderPass = gbuffer_common->gbuffer_vulkan.render_pass;
    framebuffer_info.attachmentCount = GBUFFER_TOTAL_ATTACHMENTS;
    framebuffer_info.pAttachments = attachments_framebuffer;
    framebuffer_info.width = gbuffer_width;
    framebuffer_info.height = gbuffer_height;
    framebuffer_info.layers = 1;

    if (vkCreateFramebuffer(api_common->vulkan_api.device, &framebuffer_info, NULL, &(gbuffer_common->gbuffer_vulkan.framebuffer)) != VK_SUCCESS) {
      log_message(LOG_SEVERITY_ERROR, "Failed to create framebuffer!\n");
      return VULKAN_RENDERER_CREATE_FRAME_BUFFER_ERROR;
    }
  }

  return 0;
}

u8 gbuffer_vulkan_init(struct GBufferCommon* gbuffer_common, struct APICommon* api_common, struct SwapChainCommon* swap_chain_common, const uint_fast32_t msaa_samples) {
  if (gbuffer_vulkan_init_common(gbuffer_common, api_common, swap_chain_common, msaa_samples) != 0)
    return 1;

  if (msaa_samples != 1) {
    VkSemaphoreCreateInfo semaphore_info;
    memset(&semaphore_info, 0, sizeof(semaphore_info));
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    vkCreateSemaphore(api_common->vulkan_api.device, &semaphore_info, NULL, &(gbuffer_common->gbuffer_vulkan.semaphore));
    vulkan_graphics_utils_create_sampler(api_common->vulkan_api.device, &(gbuffer_common->gbuffer_vulkan.texture_sampler), (struct SamplerSettings){.mip_levels = 1, .min_filter = VK_FILTER_LINEAR, .mag_filter = VK_FILTER_LINEAR, .mipmap_mode = VK_SAMPLER_MIPMAP_MODE_NEAREST, .address_mode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, .anisotropy_enable = VK_FALSE, .max_anisotropy = 1.0f});
    gbuffer_common->projection_matrix = MAT4_ZERO;
    gbuffer_common->view_matrix = MAT4_ZERO;

    // Gbuffer command buffer
    VkCommandBufferAllocateInfo alloc_info_offscreen;
    memset(&alloc_info_offscreen, 0, sizeof(alloc_info_offscreen));
    alloc_info_offscreen.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info_offscreen.commandPool = api_common->vulkan_api.command_pool;
    alloc_info_offscreen.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info_offscreen.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(api_common->vulkan_api.device, &alloc_info_offscreen, &(gbuffer_common->gbuffer_vulkan.command_buffer)) != VK_SUCCESS) {
      log_message(LOG_SEVERITY_ERROR, "Failed to begin recording command buffer!\n");
      return VULKAN_RENDERER_CREATE_COMMAND_BUFFER_ERROR;
    }
  } else {
    VkSemaphoreCreateInfo semaphore_info;
    memset(&semaphore_info, 0, sizeof(semaphore_info));
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    vkCreateSemaphore(api_common->vulkan_api.device, &semaphore_info, NULL, &(gbuffer_common->gbuffer_vulkan.semaphore));
    vulkan_graphics_utils_create_sampler(api_common->vulkan_api.device, &(gbuffer_common->gbuffer_vulkan.texture_sampler), (struct SamplerSettings){.mip_levels = 1, .min_filter = VK_FILTER_LINEAR, .mag_filter = VK_FILTER_LINEAR, .mipmap_mode = VK_SAMPLER_MIPMAP_MODE_NEAREST, .address_mode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, .anisotropy_enable = VK_FALSE, .max_anisotropy = 1.0f});
    gbuffer_common->projection_matrix = MAT4_ZERO;
    gbuffer_common->view_matrix = MAT4_ZERO;

    // Gbuffer command buffer
    VkCommandBufferAllocateInfo alloc_info_offscreen;
    memset(&alloc_info_offscreen, 0, sizeof(alloc_info_offscreen));
    alloc_info_offscreen.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info_offscreen.commandPool = api_common->vulkan_api.command_pool;
    alloc_info_offscreen.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info_offscreen.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(api_common->vulkan_api.device, &alloc_info_offscreen, &(gbuffer_common->gbuffer_vulkan.command_buffer)) != VK_SUCCESS) {
      log_message(LOG_SEVERITY_ERROR, "Failed to begin recording command buffer!\n");
      return VULKAN_RENDERER_CREATE_COMMAND_BUFFER_ERROR;
    }
  }

  return 0;
}

internal inline void gbuffer_vulkan_delete_common(struct GBufferCommon* gbuffer_common, struct APICommon* api_common, const uint_fast32_t msaa_samples) {
  vkDestroyRenderPass(api_common->vulkan_api.device, gbuffer_common->gbuffer_vulkan.render_pass, NULL);

  vkDestroyFramebuffer(api_common->vulkan_api.device, gbuffer_common->gbuffer_vulkan.framebuffer, NULL);

  // Resolve
  vkDestroyImageView(api_common->vulkan_api.device, gbuffer_common->gbuffer_vulkan.color_image_view, NULL);
  vkDestroyImage(api_common->vulkan_api.device, gbuffer_common->gbuffer_vulkan.color_image, NULL);
  vkFreeMemory(api_common->vulkan_api.device, gbuffer_common->gbuffer_vulkan.color_image_memory, NULL);

  vkDestroyImageView(api_common->vulkan_api.device, gbuffer_common->gbuffer_vulkan.normal_image_view, NULL);
  vkDestroyImage(api_common->vulkan_api.device, gbuffer_common->gbuffer_vulkan.normal_image, NULL);
  vkFreeMemory(api_common->vulkan_api.device, gbuffer_common->gbuffer_vulkan.normal_image_memory, NULL);

  vkDestroyImageView(api_common->vulkan_api.device, gbuffer_common->gbuffer_vulkan.depth_image_view, NULL);
  vkDestroyImage(api_common->vulkan_api.device, gbuffer_common->gbuffer_vulkan.depth_image, NULL);
  vkFreeMemory(api_common->vulkan_api.device, gbuffer_common->gbuffer_vulkan.depth_image_memory, NULL);

  if (msaa_samples != 1) {
    // Multisample
    vkDestroyImageView(api_common->vulkan_api.device, gbuffer_common->gbuffer_vulkan.multisample_color_image_view, NULL);
    vkDestroyImage(api_common->vulkan_api.device, gbuffer_common->gbuffer_vulkan.multisample_color_image, NULL);
    vkFreeMemory(api_common->vulkan_api.device, gbuffer_common->gbuffer_vulkan.multisample_color_image_memory, NULL);

    vkDestroyImageView(api_common->vulkan_api.device, gbuffer_common->gbuffer_vulkan.multisample_normal_image_view, NULL);
    vkDestroyImage(api_common->vulkan_api.device, gbuffer_common->gbuffer_vulkan.multisample_normal_image, NULL);
    vkFreeMemory(api_common->vulkan_api.device, gbuffer_common->gbuffer_vulkan.multisample_normal_image_memory, NULL);
  }
}

void gbuffer_vulkan_delete(struct GBufferCommon* gbuffer_common, struct APICommon* api_common, const uint_fast32_t msaa_samples) {
  gbuffer_vulkan_delete_common(gbuffer_common, api_common, msaa_samples);

  vkDestroySemaphore(api_common->vulkan_api.device, gbuffer_common->gbuffer_vulkan.semaphore, NULL);
  vkDestroySampler(api_common->vulkan_api.device, gbuffer_common->gbuffer_vulkan.texture_sampler, NULL);
}

u8 gbuffer_vulkan_resize(struct GBufferCommon* gbuffer_common, struct APICommon* api_common, struct SwapChainCommon* swap_chain_common, const uint_fast32_t msaa_samples) {
  gbuffer_vulkan_delete_common(gbuffer_common, api_common, msaa_samples);
  gbuffer_vulkan_init_common(gbuffer_common, api_common, swap_chain_common, msaa_samples);

  return 0;
}

u8 gbuffer_vulkan_start(struct GBufferCommon* gbuffer_common, struct SwapChainCommon* swap_chain_common, const uint_fast32_t msaa_samples) {
  VkCommandBufferBeginInfo begin_info;
  memset(&begin_info, 0, sizeof(begin_info));
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

  if (vkBeginCommandBuffer(gbuffer_common->gbuffer_vulkan.command_buffer, &begin_info) != VK_SUCCESS) {
    log_message(LOG_SEVERITY_ERROR, "Failed to begin recording command buffer!\n");
    return VULKAN_RENDERER_CREATE_COMMAND_BUFFER_ERROR;
  }

  VkRenderPassBeginInfo render_pass_info;
  memset(&render_pass_info, 0, sizeof(render_pass_info));
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  render_pass_info.renderPass = gbuffer_common->gbuffer_vulkan.render_pass;
  render_pass_info.framebuffer = gbuffer_common->gbuffer_vulkan.framebuffer;
  render_pass_info.renderArea.offset.x = 0;
  render_pass_info.renderArea.offset.y = 0;
  render_pass_info.renderArea.extent.width = (u32)((r32)swap_chain_common->swap_chain_extent.width * swap_chain_common->supersample_scale);
  render_pass_info.renderArea.extent.height = (u32)((r32)swap_chain_common->swap_chain_extent.height * swap_chain_common->supersample_scale);

  if (msaa_samples != 1) {
    VkClearValue clear_values[MULTISAMPLE_GBUFFER_TOTAL_ATTACHMENTS] = {0};

    // Should not be clearing color only normals and depth
    // Faster to just draw over old color content instead of clearing
    // http://ogldev.atspace.co.uk/www/tutorial51/tutorial51.html
    // VkClearColorValue multisample_clear_color = {{0.0f, 0.0f, 0.0f, 0.0f}};
    VkClearColorValue multisample_clear_color = {{gbuffer_common->color_clear_value.r, gbuffer_common->color_clear_value.g, gbuffer_common->color_clear_value.b, gbuffer_common->color_clear_value.a}};
    clear_values[0].color = multisample_clear_color;

    VkClearColorValue multisample_clear_normals = {{gbuffer_common->normal_clear_value.r, gbuffer_common->normal_clear_value.g, gbuffer_common->normal_clear_value.b, gbuffer_common->normal_clear_value.a}};
    clear_values[1].color = multisample_clear_normals;

    VkClearDepthStencilValue clear_depth = {gbuffer_common->depth_clear_value, 0};
    clear_values[2].depthStencil = clear_depth;

    // VkClearColorValue clear_color = {{0.0f, 0.0f, 0.0f, 0.0f}};
    VkClearColorValue clear_color = {{gbuffer_common->color_clear_value.r, gbuffer_common->color_clear_value.g, gbuffer_common->color_clear_value.b, gbuffer_common->color_clear_value.a}};
    clear_values[3].color = clear_color;

    VkClearColorValue clear_normals = {{gbuffer_common->normal_clear_value.r, gbuffer_common->normal_clear_value.g, gbuffer_common->normal_clear_value.b, gbuffer_common->normal_clear_value.a}};
    clear_values[4].color = clear_normals;

    render_pass_info.clearValueCount = MULTISAMPLE_GBUFFER_TOTAL_ATTACHMENTS;
    render_pass_info.pClearValues = clear_values;

    vkCmdBeginRenderPass(gbuffer_common->gbuffer_vulkan.command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
  } else {
    VkClearValue clear_values[GBUFFER_TOTAL_ATTACHMENTS] = {0};

    // VkClearColorValue clear_color = {{0.0f, 0.0f, 0.0f, 0.0f}};
    VkClearColorValue clear_color = {{gbuffer_common->color_clear_value.r, gbuffer_common->color_clear_value.g, gbuffer_common->color_clear_value.b, gbuffer_common->color_clear_value.a}};
    clear_values[0].color = clear_color;

    VkClearColorValue clear_normals = {{gbuffer_common->normal_clear_value.r, gbuffer_common->normal_clear_value.g, gbuffer_common->normal_clear_value.b, gbuffer_common->normal_clear_value.a}};
    clear_values[1].color = clear_normals;

    VkClearDepthStencilValue clear_depth = {gbuffer_common->depth_clear_value, 0};
    clear_values[2].depthStencil = clear_depth;

    render_pass_info.clearValueCount = GBUFFER_TOTAL_ATTACHMENTS;
    render_pass_info.pClearValues = clear_values;

    vkCmdBeginRenderPass(gbuffer_common->gbuffer_vulkan.command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
  }

  return 0;
}

u8 gbuffer_vulkan_stop(struct GBufferCommon* gbuffer_common, struct APICommon* api_common, const uint_fast32_t msaa_samples) {
  vkCmdEndRenderPass(gbuffer_common->gbuffer_vulkan.command_buffer);

  if (vkEndCommandBuffer(gbuffer_common->gbuffer_vulkan.command_buffer) != VK_SUCCESS) {
    log_message(LOG_SEVERITY_ERROR, "Failed to begin recording command buffer!\n");
    return VULKAN_RENDERER_CREATE_COMMAND_BUFFER_ERROR;
  }

  // Send to GPU for offscreen rendering then wait until finished
  VkSubmitInfo gbuffer_submit_info;
  memset(&gbuffer_submit_info, 0, sizeof(gbuffer_submit_info));
  gbuffer_submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  gbuffer_submit_info.commandBufferCount = 1;
  gbuffer_submit_info.pCommandBuffers = &(gbuffer_common->gbuffer_vulkan.command_buffer);
  VkSemaphore signal_semaphores[] = {gbuffer_common->gbuffer_vulkan.semaphore};
  gbuffer_submit_info.signalSemaphoreCount = 1;
  gbuffer_submit_info.pSignalSemaphores = signal_semaphores;

  VkResult result = vkQueueSubmit(api_common->vulkan_api.graphics_queue, 1, &gbuffer_submit_info, VK_NULL_HANDLE);

  if (result != VK_SUCCESS)
    fprintf(stderr, "Error to submit GBuffer!\n");

  return 0;
}
