#include "mana/graphics/utilities/texture/texturevulkan.h"

uint8_t texture_vulkan_init(struct TextureCommon* texture_common, struct TextureManagerCommon* texture_manager_common, struct APICommon* api_common, struct TextureSettings* texture_settings, void* pixels) {
  VkFilter filter = (texture_settings->filter_type == FILTER_NEAREST) ? VK_FILTER_NEAREST : VK_FILTER_LINEAR;
  VkSamplerAddressMode mode;
  switch (texture_settings->mode_type) {
    case (MODE_REPEAT): {
      mode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
      break;
    }
    case (MODE_MIRRORED_REPEAT): {
      mode = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
      break;
    }
    case (MODE_CLAMP_TO_EDGE): {
      mode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
      break;
    }
    case (MODE_CLAMP_TO_BORDER): {
      mode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
      break;
    }
  }

  uint8_t bytes_per_channel = 1;
  uint8_t channels = 4;
  VkFormat format = VK_FORMAT_UNDEFINED;
  switch (texture_settings->format_type) {
    case (FORMAT_R8_UNORM): {
      format = VK_FORMAT_R8_UNORM;
      bytes_per_channel = 1;
      channels = 1;
      break;
    }
    case (FORMAT_R8G8B8A8_UNORM): {
      format = VK_FORMAT_R8G8B8A8_UNORM;
      bytes_per_channel = 1;
      channels = 4;
      break;
    }
    case (FORMAT_R16_UNORM): {
      format = VK_FORMAT_R16_UNORM;
      bytes_per_channel = 2;
      channels = 1;
      break;
    }
    case (FORMAT_R16G16B16A16_UNORM): {
      format = VK_FORMAT_R16G16B16A16_UNORM;
      bytes_per_channel = 2;
      channels = 4;
      break;
    }
    case (FORMAT_R32_SFLOAT): {
      format = VK_FORMAT_R32_SFLOAT;
      bytes_per_channel = 4;
      channels = 1;
      break;
    }
    case (FORMAT_R32G32B32A32_SFLOAT): {
      format = VK_FORMAT_R32G32B32A32_SFLOAT;
      bytes_per_channel = 4;
      channels = 4;
      break;
    }
  }

  uint32_t mip_levels = 1;
  if (texture_settings->mip_type == MIP_GENERATE)
    mip_levels = (uint32_t)(floor(log2(MAX(texture_common->width, texture_common->height)))) + 1;
  else if (texture_settings->mip_type == MIP_CUSTOM)
    mip_levels = texture_settings->mip_count;

  VkDeviceSize image_size = 0;
  if (texture_settings->mip_type == MIP_CUSTOM) {
    for (uint32_t level = 0; level < mip_levels; level++) {
      uint32_t w = texture_common->width >> level;
      if (!w)
        w = 1;
      uint32_t h = texture_common->height >> level;
      if (!h)
        h = 1;
      image_size += (VkDeviceSize)w * h * channels * bytes_per_channel;
    }
  } else
    image_size = (VkDeviceSize)texture_common->width * texture_common->height * channels * bytes_per_channel;

  VkBuffer staging_buffer = {0};
  VkDeviceMemory staging_buffer_memory = {0};
  vulkan_graphics_utils_create_buffer(api_common->vulkan_api.device, api_common->vulkan_api.physical_device, image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging_buffer, &staging_buffer_memory);

  void* data;
  vkMapMemory(api_common->vulkan_api.device, staging_buffer_memory, 0, image_size, 0, &data);
  memcpy(data, pixels, image_size);
  vkUnmapMemory(api_common->vulkan_api.device, staging_buffer_memory);

  vulkan_graphics_utils_create_image(api_common->vulkan_api.device, api_common->vulkan_api.physical_device, texture_common->width, texture_common->height, mip_levels, VK_SAMPLE_COUNT_1_BIT, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &(texture_common->texture_vulkan.texture_image), &(texture_common->texture_vulkan.texture_image_memory));

  vulkan_graphics_utils_transition_image_layout(api_common->vulkan_api.device, api_common->vulkan_api.graphics_queue, api_common->vulkan_api.command_pool, texture_common->texture_vulkan.texture_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mip_levels);
  if (texture_settings->mip_type == MIP_CUSTOM) {
    VkCommandBuffer cmd = vulkan_graphics_utils_begin_single_time_commands(api_common->vulkan_api.device, api_common->vulkan_api.command_pool);
    VkDeviceSize offset = 0;

    for (uint32_t level = 0; level < mip_levels; level++) {
      uint32_t w = texture_common->width >> level;
      uint32_t h = texture_common->height >> level;

      if (w == 0) w = 1;
      if (h == 0) h = 1;

      VkBufferImageCopy region = {0};
      region.bufferOffset = offset;
      region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      region.imageSubresource.mipLevel = level;
      region.imageSubresource.baseArrayLayer = 0;
      region.imageSubresource.layerCount = 1;
      region.imageExtent.width = w;
      region.imageExtent.height = h;
      region.imageExtent.depth = 1;
      region.bufferRowLength = 0;
      region.bufferImageHeight = 0;
      region.imageOffset = (VkOffset3D){0, 0, 0};

      vkCmdCopyBufferToImage(cmd, staging_buffer, texture_common->texture_vulkan.texture_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

      offset += (VkDeviceSize)w * h * channels * bytes_per_channel;
    }

    vulkan_graphics_utils_end_single_time_commands(api_common->vulkan_api.device, api_common->vulkan_api.graphics_queue, api_common->vulkan_api.command_pool, cmd);
    vulkan_graphics_utils_transition_image_layout(api_common->vulkan_api.device, api_common->vulkan_api.graphics_queue, api_common->vulkan_api.command_pool, texture_common->texture_vulkan.texture_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mip_levels);
  } else
    vulkan_graphics_utils_copy_buffer_to_image(api_common->vulkan_api.device, api_common->vulkan_api.graphics_queue, api_common->vulkan_api.command_pool, &staging_buffer, &(texture_common->texture_vulkan.texture_image), texture_common->width, texture_common->height);

  vkDestroyBuffer(api_common->vulkan_api.device, staging_buffer, NULL);
  vkFreeMemory(api_common->vulkan_api.device, staging_buffer_memory, NULL);

  if (texture_settings->mip_type == MIP_GENERATE)
    vulkan_graphics_utils_generate_mipmaps(api_common->vulkan_api.device, api_common->vulkan_api.physical_device, api_common->vulkan_api.graphics_queue, api_common->vulkan_api.command_pool, texture_common->texture_vulkan.texture_image, format, texture_common->width, texture_common->height, mip_levels);

  vulkan_graphics_utils_create_image_view(api_common->vulkan_api.device, texture_common->texture_vulkan.texture_image, format, VK_IMAGE_ASPECT_COLOR_BIT, mip_levels, &(texture_common->texture_vulkan.texture_image_view));
  vulkan_graphics_utils_create_sampler(api_common->vulkan_api.device, &(texture_common->texture_vulkan.texture_sampler), (struct SamplerSettings){.mip_levels = mip_levels, .filter = filter, .address_mode = mode});

  return 0;
}

void texture_vulkan_delete(struct TextureCommon* texture_common, struct APICommon* api_common) {
  vkDestroySampler(api_common->vulkan_api.device, texture_common->texture_vulkan.texture_sampler, NULL);
  vkDestroyImageView(api_common->vulkan_api.device, texture_common->texture_vulkan.texture_image_view, NULL);

  vkDestroyImage(api_common->vulkan_api.device, texture_common->texture_vulkan.texture_image, NULL);
  vkFreeMemory(api_common->vulkan_api.device, texture_common->texture_vulkan.texture_image_memory, NULL);
}
