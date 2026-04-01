#include "mana/graphics/render/swapchain/swapchainvulkan.h"

internal inline u8 swap_chain_vulkan_init_common(struct SwapChainCommon* swap_chain_common, struct APICommon* api_common, uint_fast32_t width, uint_fast32_t height, VkSwapchainKHR* old_swap_chain) {
  struct SwapChainSupportDetails swap_chain_support = {0};

  vector_init(&swap_chain_support.formats, sizeof(struct VkSurfaceFormatKHR));
  vector_init(&swap_chain_support.present_modes, sizeof(enum VkPresentModeKHR));

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(api_common->vulkan_api.physical_device, swap_chain_common->swap_chain_vulkan.surface, &swap_chain_support.capabilities);

  u32 format_count;
  vkGetPhysicalDeviceSurfaceFormatsKHR(api_common->vulkan_api.physical_device, swap_chain_common->swap_chain_vulkan.surface, &format_count, NULL);

  if (format_count != 0) {
    vector_resize(&swap_chain_support.formats, format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(api_common->vulkan_api.physical_device, swap_chain_common->swap_chain_vulkan.surface, &format_count, (VkSurfaceFormatKHR*)swap_chain_support.formats.items);
    swap_chain_support.formats.size = format_count;
  }

  u32 present_mode_count;
  vkGetPhysicalDeviceSurfacePresentModesKHR(api_common->vulkan_api.physical_device, swap_chain_common->swap_chain_vulkan.surface, &present_mode_count, NULL);

  if (present_mode_count != 0) {
    vector_resize(&swap_chain_support.present_modes, present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(api_common->vulkan_api.physical_device, swap_chain_common->swap_chain_vulkan.surface, &present_mode_count, (VkPresentModeKHR*)swap_chain_support.present_modes.items);
    swap_chain_support.present_modes.size = present_mode_count;
  }

  VkSurfaceFormatKHR surface_format;
  memset(&surface_format, 0, sizeof(surface_format));

  if (format_count == 1 && ((struct VkSurfaceFormatKHR*)vector_get(&swap_chain_support.formats, 0))->format == VK_FORMAT_UNDEFINED) {
    surface_format.format = VK_FORMAT_B8G8R8A8_UNORM;
    surface_format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
  } else {
    for (size_t swap_chain_format_num = 0; swap_chain_format_num < vector_size(&swap_chain_support.formats); swap_chain_format_num++) {
      if (((struct VkSurfaceFormatKHR*)vector_get(&swap_chain_support.formats, swap_chain_format_num))->format == VK_FORMAT_B8G8R8A8_UNORM && ((struct VkSurfaceFormatKHR*)vector_get(&swap_chain_support.formats, swap_chain_format_num))->colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
        surface_format = *(struct VkSurfaceFormatKHR*)vector_get(&swap_chain_support.formats, swap_chain_format_num);
        break;
      } else
        surface_format = *(struct VkSurfaceFormatKHR*)vector_get(&swap_chain_support.formats, 0);
    }
  }

  // TODO: Clean up and let user select 8 bit or 10 bit
  // surface_format.format = VK_FORMAT_A2B10G10R10_UNORM_PACK32;
  // Note: I only need RGB NOT A for game. A channel would be for overlays
  // So I should just use 888 or 161616 for games
  // That and see what formats the swapchain normally supports to get it right
  // NOTE: VK_FORMAT_A2B10G10R10_UNORM_PACK32 is found as a viable format so it seems offical 10 bit support here for swapchain
  // VK_FORMAT_A2B10G10R10_UNORM_PACK32 is useful because
  // VK_FORMAT_R16G16B16_UNORM
  // Hmm looks like I should loop through, rank them, and generally pick the first one
  // If I need more color depth experiment with 16 or 10 bit, should support up to 12 bit monitors at least

  VkPresentModeKHR present_mode;
  memset(&present_mode, 0, sizeof(present_mode));

  for (size_t swap_chain_present_num = 0; swap_chain_present_num < vector_size(&swap_chain_support.present_modes); swap_chain_present_num++) {
    if (*(enum VkPresentModeKHR*)vector_get(&swap_chain_support.present_modes, swap_chain_present_num) == VK_PRESENT_MODE_MAILBOX_KHR) {
      present_mode = *(enum VkPresentModeKHR*)vector_get(&swap_chain_support.present_modes, swap_chain_present_num);
      break;
    } else if (*(enum VkPresentModeKHR*)vector_get(&swap_chain_support.present_modes, swap_chain_present_num) == VK_PRESENT_MODE_IMMEDIATE_KHR)
      present_mode = *(enum VkPresentModeKHR*)vector_get(&swap_chain_support.present_modes, swap_chain_present_num);
  }

  // VK_PRESENT_MODE_FIFO_KHR is generic high compatibility vsync
  // VK_PRESENT_MODE_MAILBOX_KHR is vsync with triple buffering ideal for gaming because lower latency, though doesn't wait for frames so no fps cap because no stalling occurs
  // present_mode = VK_PRESENT_MODE_FIFO_KHR;
  if (swap_chain_common->vsync)
    present_mode = VK_PRESENT_MODE_FIFO_KHR;
  else
    present_mode = VK_PRESENT_MODE_MAILBOX_KHR;

  VkExtent2D extent = {width, height};
  if (swap_chain_support.capabilities.currentExtent.width != UINT32_MAX)
    extent = swap_chain_support.capabilities.currentExtent;
  else {
    extent.width = MAX(swap_chain_support.capabilities.minImageExtent.width, MIN(swap_chain_support.capabilities.maxImageExtent.width, extent.width));
    extent.height = MAX(swap_chain_support.capabilities.minImageExtent.height, MIN(swap_chain_support.capabilities.maxImageExtent.height, extent.height));
  }

  // NOTE: Possible adding to high gpu usage when chilling?
  u32 image_count = swap_chain_support.capabilities.minImageCount + 1;
  if (swap_chain_support.capabilities.maxImageCount > 0 && image_count > swap_chain_support.capabilities.maxImageCount)
    image_count = swap_chain_support.capabilities.maxImageCount;

  VkSwapchainCreateInfoKHR swapchain_info;
  memset(&swapchain_info, 0, sizeof(swapchain_info));
  swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapchain_info.surface = swap_chain_common->swap_chain_vulkan.surface;
  swapchain_info.minImageCount = image_count;
  swapchain_info.imageFormat = surface_format.format;
  swapchain_info.imageColorSpace = surface_format.colorSpace;
  swapchain_info.imageExtent = extent;
  swapchain_info.imageArrayLayers = 1;
  swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  if (old_swap_chain != NULL)
    swapchain_info.oldSwapchain = *old_swap_chain;

  u32 queue_family_indices[] = {(&api_common->vulkan_api.indices)->graphics_family, (&api_common->vulkan_api.indices)->present_family};

  if ((&api_common->vulkan_api.indices)->graphics_family != (&api_common->vulkan_api.indices)->present_family) {
    swapchain_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    swapchain_info.queueFamilyIndexCount = 2;
    swapchain_info.pQueueFamilyIndices = queue_family_indices;
  } else
    swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

  swapchain_info.preTransform = swap_chain_support.capabilities.currentTransform;
  swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapchain_info.presentMode = present_mode;
  swapchain_info.clipped = VK_TRUE;
  //
  VkFormatProperties format_props;
  vkGetPhysicalDeviceFormatProperties(api_common->vulkan_api.physical_device, surface_format.format, &format_props);

  if (format_props.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) {
    // The format supports linear tiling for sampled images
  }

  if (format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) {
    // The format supports optimal tiling for sampled images
  }

  VkImageFormatProperties format_props_2;
  VkResult res = vkGetPhysicalDeviceImageFormatProperties(api_common->vulkan_api.physical_device, surface_format.format, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 0, &format_props_2);

  if (res == VK_SUCCESS) {
    // The image format is supported with the specified tiling mode
  } else {
    // The image format is not supported with the specified tiling mode
  }
  //
  if (vkCreateSwapchainKHR(api_common->vulkan_api.device, &swapchain_info, NULL, &(swap_chain_common->swap_chain_vulkan.swap_chain_khr)) != VK_SUCCESS) {
    log_message(LOG_SEVERITY_ERROR, "Failed to create swap chain!\n");
    return VULKAN_RENDERER_CREATE_SWAP_CHAIN_ERROR;
  }

  vkGetSwapchainImagesKHR(api_common->vulkan_api.device, swap_chain_common->swap_chain_vulkan.swap_chain_khr, &image_count, NULL);
  swap_chain_common->swap_chain_vulkan.swap_chain_image_count = image_count;
  vkGetSwapchainImagesKHR(api_common->vulkan_api.device, swap_chain_common->swap_chain_vulkan.swap_chain_khr, &image_count, swap_chain_common->swap_chain_vulkan.swap_chain_images);

  swap_chain_common->swap_chain_vulkan.swap_chain_image_format = surface_format.format;

  vector_delete(&swap_chain_support.formats);
  vector_delete(&swap_chain_support.present_modes);

  ////////////////////////////////////////////////////////

  // TODO: Clean this up like GBuffer
  for (size_t i = 0; i < MAX_SWAP_CHAIN_FRAMES; i++) {
    VkImageViewCreateInfo view_info;
    memset(&view_info, 0, sizeof(view_info));
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = swap_chain_common->swap_chain_vulkan.swap_chain_images[i];
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = swap_chain_common->swap_chain_vulkan.swap_chain_image_format;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    if (vkCreateImageView(api_common->vulkan_api.device, &view_info, NULL, (&swap_chain_common->swap_chain_vulkan.swap_chain_image_views[i])) != VK_SUCCESS) {
      log_message(LOG_SEVERITY_ERROR, "Failed to create image views!\n");
      return VULKAN_RENDERER_CREATE_IMAGE_VIEWS_ERROR;
    }
  }

  struct VkAttachmentDescription color_attachment;
  memset(&color_attachment, 0, sizeof(color_attachment));
  vulkan_graphics_utils_create_color_attachment(swap_chain_common->swap_chain_vulkan.swap_chain_image_format, &color_attachment);
  // color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;

  VkAttachmentReference color_attachment_ref;
  memset(&color_attachment_ref, 0, sizeof(color_attachment_ref));
  color_attachment_ref.attachment = 0;
  color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  struct VkAttachmentReference color_attachment_references = color_attachment_ref;
  VkSubpassDescription subpass = {0};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_attachment_references;

  VkSubpassDependency dependency;
  memset(&dependency, 0, sizeof(dependency));
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  VkRenderPassCreateInfo render_pass_info;
  memset(&render_pass_info, 0, sizeof(render_pass_info));
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  render_pass_info.attachmentCount = 1;
  render_pass_info.pAttachments = &color_attachment;
  render_pass_info.subpassCount = 1;
  render_pass_info.pSubpasses = &subpass;
  render_pass_info.dependencyCount = 1;
  render_pass_info.pDependencies = &dependency;

  if (vkCreateRenderPass(api_common->vulkan_api.device, &render_pass_info, NULL, &(swap_chain_common->swap_chain_vulkan.render_pass)) != VK_SUCCESS)
    return 0;

  for (int loop_num = 0; loop_num < MAX_SWAP_CHAIN_FRAMES; loop_num++) {
    VkFramebufferCreateInfo framebuffer_info;
    memset(&framebuffer_info, 0, sizeof(framebuffer_info));
    framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_info.renderPass = swap_chain_common->swap_chain_vulkan.render_pass;
    framebuffer_info.attachmentCount = 1;
    framebuffer_info.pAttachments = &(swap_chain_common->swap_chain_vulkan.swap_chain_image_views[loop_num]);
    framebuffer_info.width = swap_chain_common->swap_chain_extent.width;
    framebuffer_info.height = swap_chain_common->swap_chain_extent.height;
    framebuffer_info.layers = 1;

    if (vkCreateFramebuffer(api_common->vulkan_api.device, &framebuffer_info, NULL, &(swap_chain_common->swap_chain_vulkan.swap_chain_framebuffers[loop_num])) != VK_SUCCESS) {
      log_message(LOG_SEVERITY_ERROR, "Failed to create framebuffer!\n");
      return VULKAN_RENDERER_CREATE_FRAME_BUFFER_ERROR;
    }
  }

  return 0;
}

internal inline void swap_chain_vulkan_update_uniform_buffer(struct SwapChainCommon* swap_chain_common, struct APICommon* api_common, uint_fast32_t width, uint_fast32_t height) {
  struct BlitUniformBufferObject ubos = {.screen_size = (vec2){.x = (r32)swap_chain_common->swap_chain_extent.width, .y = (r32)swap_chain_common->swap_chain_extent.height}};
  void* data;
  vkMapMemory(api_common->vulkan_api.device, swap_chain_common->swap_chain_vulkan.uniform_buffers_memory, 0, sizeof(struct BlitUniformBufferObject), 0, &data);
  memcpy(data, &ubos, sizeof(struct BlitUniformBufferObject));
  vkUnmapMemory(api_common->vulkan_api.device, swap_chain_common->swap_chain_vulkan.uniform_buffers_memory);
}

u8 swap_chain_vulkan_init(struct SwapChainCommon* swap_chain_common, struct APICommon* api_common, uint_fast32_t width, uint_fast32_t height, b8 vsync, void* extra_data) {
  if (swap_chain_vulkan_init_common(swap_chain_common, api_common, width, height, NULL) != 0)
    return VULKAN_RENDERER_CREATE_SWAP_CHAIN_ERROR;

  // Swapchain command buffer
  VkCommandBufferAllocateInfo alloc_info_swapchain;
  memset(&alloc_info_swapchain, 0, sizeof(alloc_info_swapchain));
  alloc_info_swapchain.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info_swapchain.commandPool = api_common->vulkan_api.command_pool;
  alloc_info_swapchain.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info_swapchain.commandBufferCount = (u32)MAX_SWAP_CHAIN_FRAMES;

  if (vkAllocateCommandBuffers(api_common->vulkan_api.device, &alloc_info_swapchain, swap_chain_common->swap_chain_vulkan.swap_chain_command_buffers) != VK_SUCCESS) {
    log_message(LOG_SEVERITY_ERROR, "Failed to begin recording command buffer!\n");
    return VULKAN_RENDERER_CREATE_COMMAND_BUFFER_ERROR;
  }

  memset(swap_chain_common->swap_chain_vulkan.in_flight_fences, 0, sizeof(swap_chain_common->swap_chain_vulkan.in_flight_fences));

  VkSemaphoreCreateInfo semaphore_info;
  memset(&semaphore_info, 0, sizeof(semaphore_info));
  semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fence_info;
  memset(&fence_info, 0, sizeof(fence_info));
  fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    if (vkCreateSemaphore(api_common->vulkan_api.device, &semaphore_info, NULL, &swap_chain_common->swap_chain_vulkan.image_available_semaphores[i]) != VK_SUCCESS || vkCreateFence(api_common->vulkan_api.device, &fence_info, NULL, &swap_chain_common->swap_chain_vulkan.in_flight_fences[i]) != VK_SUCCESS) {
      log_message(LOG_SEVERITY_ERROR, "Failed to create per-frame sync objects!\n");
      return VULKAN_RENDERER_CREATE_SYNC_OBJECT_ERROR;
    }
  }

  for (u32 i = 0; i < swap_chain_common->swap_chain_vulkan.swap_chain_image_count; i++) {
    if (vkCreateSemaphore(api_common->vulkan_api.device, &semaphore_info, NULL, &swap_chain_common->swap_chain_vulkan.render_finished_semaphores[i]) != VK_SUCCESS) {
      log_message(LOG_SEVERITY_ERROR, "Failed to create per-image render-finished semaphores!\n");
      return VULKAN_RENDERER_CREATE_SYNC_OBJECT_ERROR;
    }
  }

  vulkan_graphics_utils_setup_uniform_buffer(&(api_common->vulkan_api), sizeof(struct BlitUniformBufferObject), &(swap_chain_common->swap_chain_vulkan.uniform_buffer), &(swap_chain_common->swap_chain_vulkan.uniform_buffers_memory));
  swap_chain_vulkan_update_uniform_buffer(swap_chain_common, api_common, width, height);

  swap_chain_common->blit_shader->extra_data = swap_chain_common->swap_chain_vulkan.render_pass;

  return 0;
}

internal inline void swap_chain_vulkan_delete_common(struct SwapChainCommon* swap_chain_common, struct APICommon* api_common) {
  vkDestroyRenderPass(api_common->vulkan_api.device, swap_chain_common->swap_chain_vulkan.render_pass, NULL);

  for (u8 loop_num = 0; loop_num < MAX_SWAP_CHAIN_FRAMES; loop_num++) {
    vkDestroyFramebuffer(api_common->vulkan_api.device, swap_chain_common->swap_chain_vulkan.swap_chain_framebuffers[loop_num], NULL);
    vkDestroyImageView(api_common->vulkan_api.device, swap_chain_common->swap_chain_vulkan.swap_chain_image_views[loop_num], NULL);
  }
}

void swap_chain_vulkan_delete(struct SwapChainCommon* swap_chain_common, struct APICommon* api_common) {
  VkDevice device = api_common->vulkan_api.device;

  vkDeviceWaitIdle(device);

  vkDestroyBuffer(device, swap_chain_common->swap_chain_vulkan.uniform_buffer, NULL);
  vkFreeMemory(device, swap_chain_common->swap_chain_vulkan.uniform_buffers_memory, NULL);

  swap_chain_vulkan_delete_common(swap_chain_common, api_common);

  vkDestroyBuffer(device, swap_chain_common->swap_chain_vulkan.vertex_buffer, NULL);
  vkFreeMemory(device, swap_chain_common->swap_chain_vulkan.vertex_buffer_memory, NULL);

  vkDestroyBuffer(device, swap_chain_common->swap_chain_vulkan.index_buffer, NULL);
  vkFreeMemory(device, swap_chain_common->swap_chain_vulkan.index_buffer_memory, NULL);

  vkDestroySwapchainKHR(device, swap_chain_common->swap_chain_vulkan.swap_chain_khr, NULL);

  vkFreeCommandBuffers(device, api_common->vulkan_api.command_pool, MAX_SWAP_CHAIN_FRAMES, swap_chain_common->swap_chain_vulkan.swap_chain_command_buffers);

  // Per swap chain image
  for (u32 render_finished_semaphore_index = 0; render_finished_semaphore_index < swap_chain_common->swap_chain_vulkan.swap_chain_image_count; render_finished_semaphore_index++)
    vkDestroySemaphore(device, swap_chain_common->swap_chain_vulkan.render_finished_semaphores[render_finished_semaphore_index], NULL);

  // Per frame in flight
  for (u32 image_available_semaphore_index = 0; image_available_semaphore_index < MAX_FRAMES_IN_FLIGHT; image_available_semaphore_index++) {
    vkDestroySemaphore(device, swap_chain_common->swap_chain_vulkan.image_available_semaphores[image_available_semaphore_index], NULL);
    vkDestroyFence(device, swap_chain_common->swap_chain_vulkan.in_flight_fences[image_available_semaphore_index], NULL);
  }
}

u8 swap_chain_vulkan_resize(struct SwapChainCommon* swap_chain_common, struct APICommon* api_common) {
  // Ensure all operations using the swap chain have finished
  for (u8 loop_num = 0; loop_num < MAX_FRAMES_IN_FLIGHT; loop_num++)
    swap_chain_vulkan_wait_for_fences(swap_chain_common, api_common, loop_num);

  // Cleanup framebuffers and image views
  swap_chain_vulkan_delete_common(swap_chain_common, api_common);

  // Store the old swap chain
  VkSwapchainKHR old_swap_chain = swap_chain_common->swap_chain_vulkan.swap_chain_khr;

  swap_chain_vulkan_init_common(swap_chain_common, api_common, swap_chain_common->swap_chain_extent.width, swap_chain_common->swap_chain_extent.height, &old_swap_chain);

  // Destroy the old swap chain
  vkDestroySwapchainKHR(api_common->vulkan_api.device, old_swap_chain, NULL);

  swap_chain_vulkan_update_uniform_buffer(swap_chain_common, api_common, swap_chain_common->swap_chain_extent.width, swap_chain_common->swap_chain_extent.height);

  return 0;
}

void swap_chain_vulkan_prepare_delete(struct SwapChainCommon* swap_chain_common, struct APICommon* api_common) {
  // vkWaitForFences(api_common->vulkan_api.device, 2, swap_chain_common->swap_chain_vulkan.in_flight_fences, VK_TRUE, UINT64_MAX);
  vkDeviceWaitIdle(api_common->vulkan_api.device);
}

u8 swap_chain_vulkan_blit_init(struct SwapChainCommon* swap_chain_common, struct APICommon* api_common, struct PostProcessCommon* post_process_common) {
  vulkan_graphics_utils_create_descriptors(&(api_common->vulkan_api), swap_chain_common->swap_chain_vulkan.descriptor_set, &(swap_chain_common->blit_shader->shader.shader_common.shader_vulkan.descriptor_set_layout), &(swap_chain_common->blit_shader->shader.shader_common.shader_vulkan.descriptor_pool), swap_chain_common->blit_shader->shader.shader_common.shader_settings.descriptors);

  for (uint_fast64_t target = 0; target < swap_chain_common->descriptors; target++) {
    VkWriteDescriptorSet dcs[2];
    memset(dcs, 0, sizeof(dcs));
    vulkan_graphics_utils_setup_descriptor_buffer(dcs, 0, &(swap_chain_common->swap_chain_vulkan.descriptor_set[target]), (VkDescriptorBufferInfo[]){vulkan_graphics_utils_setup_descriptor_buffer_info(sizeof(struct BlitUniformBufferObject), &(swap_chain_common->swap_chain_vulkan.uniform_buffer))});
    vulkan_graphics_utils_setup_descriptor_image(dcs, 1, &(swap_chain_common->swap_chain_vulkan.descriptor_set[target]), (VkDescriptorImageInfo[]){vulkan_graphics_utils_setup_descriptor_image_info(&(post_process_common->post_process_vulkan.color_image_views[target]), &(post_process_common->post_process_vulkan.texture_sampler))});
    vkUpdateDescriptorSets(api_common->vulkan_api.device, 2, dcs, 0, NULL);
  }

  vulkan_graphics_utils_setup_vertex_buffer(&api_common->vulkan_api, swap_chain_common->blit_fullscreen_triangle.mesh_common.vertices, &(swap_chain_common->swap_chain_vulkan.vertex_buffer), &(swap_chain_common->swap_chain_vulkan.vertex_buffer_memory));
  vulkan_graphics_utils_setup_index_buffer(&api_common->vulkan_api, swap_chain_common->blit_fullscreen_triangle.mesh_common.indices, &(swap_chain_common->swap_chain_vulkan.index_buffer), &(swap_chain_common->swap_chain_vulkan.index_buffer_memory));

  return 0;
}

u8 swap_chain_vulkan_blit_update(struct SwapChainCommon* swap_chain_common, struct APICommon* api_common, struct PostProcessCommon* post_process_common) {
  for (uint_fast64_t target = 0; target < swap_chain_common->descriptors; target++) {
    VkWriteDescriptorSet dcs[2];
    memset(dcs, 0, sizeof(dcs));
    vulkan_graphics_utils_setup_descriptor_buffer(dcs, 0, &(swap_chain_common->swap_chain_vulkan.descriptor_set[target]), (VkDescriptorBufferInfo[]){vulkan_graphics_utils_setup_descriptor_buffer_info(sizeof(struct BlitUniformBufferObject), &(swap_chain_common->swap_chain_vulkan.uniform_buffer))});
    vulkan_graphics_utils_setup_descriptor_image(dcs, 1, &(swap_chain_common->swap_chain_vulkan.descriptor_set[target]), (VkDescriptorImageInfo[]){vulkan_graphics_utils_setup_descriptor_image_info(&(post_process_common->post_process_vulkan.color_image_views[target]), &(post_process_common->post_process_vulkan.texture_sampler))});
    vkUpdateDescriptorSets(api_common->vulkan_api.device, 2, dcs, 0, NULL);
  }

  return 0;
}

u8 swap_chain_vulkan_blit_render(struct SwapChainCommon* swap_chain_common, struct PostProcessCommon* post_process_common, u8 swap_chain_num) {
  struct SwapChainVulkan* swap_chain_vulkan = &(swap_chain_common->swap_chain_vulkan);

  VkCommandBufferBeginInfo begin_info;
  memset(&begin_info, 0, sizeof(begin_info));
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

  if (vkBeginCommandBuffer(swap_chain_common->swap_chain_vulkan.swap_chain_command_buffers[swap_chain_num], &begin_info) != VK_SUCCESS) {
    log_message(LOG_SEVERITY_ERROR, "Failed to begin recording command buffer!\n");
    return VULKAN_RENDERER_CREATE_COMMAND_BUFFER_ERROR;
  }

  // VkClearValue clear_color = {.color = {1.0f, 0.0f, 0.0f, 1.0f}}; // RGBA: Red
  VkRenderPassBeginInfo render_pass_info;
  memset(&render_pass_info, 0, sizeof(render_pass_info));
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  render_pass_info.renderPass = swap_chain_common->swap_chain_vulkan.render_pass;
  render_pass_info.framebuffer = swap_chain_common->swap_chain_vulkan.swap_chain_framebuffers[swap_chain_num];
  render_pass_info.renderArea.offset.x = 0;
  render_pass_info.renderArea.offset.y = 0;
  render_pass_info.renderArea.extent = (VkExtent2D){.width = swap_chain_common->swap_chain_extent.width, .height = swap_chain_common->swap_chain_extent.height};
  // render_pass_info.clearValueCount = 1;
  // render_pass_info.pClearValues = &clear_color;

  render_pass_info.clearValueCount = 0;
  render_pass_info.pClearValues = NULL;

  vkCmdBeginRenderPass(swap_chain_common->swap_chain_vulkan.swap_chain_command_buffers[swap_chain_num], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

  vkCmdBindPipeline(swap_chain_vulkan->swap_chain_command_buffers[swap_chain_num], VK_PIPELINE_BIND_POINT_GRAPHICS, swap_chain_common->blit_shader->shader.shader_common.shader_vulkan.graphics_pipeline);
  VkBuffer vertex_buffers[] = {swap_chain_common->swap_chain_vulkan.vertex_buffer};
  VkDeviceSize offsets[] = {0};
  vkCmdSetViewport(swap_chain_vulkan->swap_chain_command_buffers[swap_chain_num], 0, 1, &(swap_chain_common->blit_shader->shader.shader_common.shader_vulkan.viewport));
  vkCmdSetScissor(swap_chain_vulkan->swap_chain_command_buffers[swap_chain_num], 0, 1, &(swap_chain_common->blit_shader->shader.shader_common.shader_vulkan.scissor));
  vkCmdBindVertexBuffers(swap_chain_vulkan->swap_chain_command_buffers[swap_chain_num], 0, 1, vertex_buffers, offsets);
  vkCmdBindIndexBuffer(swap_chain_vulkan->swap_chain_command_buffers[swap_chain_num], swap_chain_common->swap_chain_vulkan.index_buffer, 0, VK_INDEX_TYPE_UINT32);
  vkCmdBindDescriptorSets(swap_chain_vulkan->swap_chain_command_buffers[swap_chain_num], VK_PIPELINE_BIND_POINT_GRAPHICS, swap_chain_common->blit_shader->shader.shader_common.shader_vulkan.pipeline_layout, 0, 1, &(swap_chain_common->swap_chain_vulkan.descriptor_set[post_process_common->ping_pong ^ TRUE]), 0, NULL);
  vkCmdDrawIndexed(swap_chain_vulkan->swap_chain_command_buffers[swap_chain_num], (u32)swap_chain_common->blit_fullscreen_triangle.mesh_common.indices->size, 1, 0, 0, 0);

  vkCmdEndRenderPass(swap_chain_common->swap_chain_vulkan.swap_chain_command_buffers[swap_chain_num]);

  if (vkEndCommandBuffer(swap_chain_common->swap_chain_vulkan.swap_chain_command_buffers[swap_chain_num]) != VK_SUCCESS) {
    log_message(LOG_SEVERITY_ERROR, "Failed to begin recording command buffer!\n");
    return VULKAN_RENDERER_CREATE_COMMAND_BUFFER_ERROR;
  }

  return 0;
}

b8 swap_chain_vulkan_wait_for_fences(struct SwapChainCommon* swap_chain_common, struct APICommon* api_common, size_t frame) {
  VkResult result = vkWaitForFences(api_common->vulkan_api.device, 2, swap_chain_common->swap_chain_vulkan.in_flight_fences, VK_TRUE, UINT64_MAX);
  result = vkAcquireNextImageKHR(api_common->vulkan_api.device, swap_chain_common->swap_chain_vulkan.swap_chain_khr, UINT64_MAX, swap_chain_common->swap_chain_vulkan.image_available_semaphores[frame], VK_NULL_HANDLE, &(swap_chain_common->image_index));

  if (result == VK_ERROR_OUT_OF_DATE_KHR)
    return TRUE;
  else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    fprintf(stderr, "failed to acquire swap chain image!\n");
    return FALSE;
  }

  return FALSE;
}

u8 swap_chain_vulkan_end_frame(struct SwapChainCommon* swap_chain_common, struct PostProcessCommon* post_process_common, struct APICommon* api_common) {
  struct VulkanAPI* vulkan_api = &api_common->vulkan_api;
  VkSubmitInfo swap_chain_submit_info;
  memset(&swap_chain_submit_info, 0, sizeof(swap_chain_submit_info));
  swap_chain_submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore wait_semaphores[VULKAN_WAIT_SEMAPHORES] = {swap_chain_common->swap_chain_vulkan.image_available_semaphores[swap_chain_common->current_frame], post_process_common->post_process_vulkan.semaphore[post_process_common->ping_pong ^ TRUE]};
  VkPipelineStageFlags wait_stages[VULKAN_WAIT_SEMAPHORES] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  swap_chain_submit_info.waitSemaphoreCount = VULKAN_WAIT_SEMAPHORES;
  swap_chain_submit_info.pWaitSemaphores = wait_semaphores;
  swap_chain_submit_info.pWaitDstStageMask = wait_stages;

  swap_chain_submit_info.commandBufferCount = 1;
  swap_chain_submit_info.pCommandBuffers = &swap_chain_common->swap_chain_vulkan.swap_chain_command_buffers[swap_chain_common->image_index];

  VkSemaphore signal_semaphores[] = {swap_chain_common->swap_chain_vulkan.render_finished_semaphores[swap_chain_common->image_index]};
  swap_chain_submit_info.signalSemaphoreCount = 1;
  swap_chain_submit_info.pSignalSemaphores = signal_semaphores;

  vkResetFences(vulkan_api->device, 1, &swap_chain_common->swap_chain_vulkan.in_flight_fences[swap_chain_common->current_frame]);

  VkResult result = vkQueueSubmit(vulkan_api->graphics_queue, 1, &swap_chain_submit_info, swap_chain_common->swap_chain_vulkan.in_flight_fences[swap_chain_common->current_frame]);

  // if (result != VK_SUCCESS)
  //   fprintf(stderr, "Error to submit draw command buffer!\n");

  VkPresentInfoKHR present_info;
  memset(&present_info, 0, sizeof(present_info));
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores = signal_semaphores;

  VkSwapchainKHR swapChains[] = {swap_chain_common->swap_chain_vulkan.swap_chain_khr};
  present_info.swapchainCount = 1;
  present_info.pSwapchains = swapChains;

  present_info.pImageIndices = &swap_chain_common->image_index;

  result = vkQueuePresentKHR(vulkan_api->present_queue, &present_info);

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    return SWAP_CHAIN_UPDATE_FRAMERBUFFER;

  return 0;
}
