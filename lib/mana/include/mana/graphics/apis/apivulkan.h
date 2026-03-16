#pragma once

#include "mana/graphics/apis/apicommon.h"

#ifdef VULKAN_API_SUPPORTED
// TODO: All these global variables should be defined or moved locally
#define VULKAN_WAIT_SEMAPHORES 2
#define VULKAN_VALIDATION_LAYER_COUNT 1
static const char* const validation_layers[VULKAN_VALIDATION_LAYER_COUNT] = {"VK_LAYER_KHRONOS_validation"};
#define VULKAN_DEVICE_EXTENSION_COUNT 1
static const char* const device_extensions[VULKAN_DEVICE_EXTENSION_COUNT] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

enum VULKAN_API_STATUS {
  VULKAN_API_SUCCESS = API_SUCCESS,
  VULKAN_API_CREATE_INSTANCE_ERROR,
  VULKAN_API_SETUP_DEBUG_MESSENGER_ERROR,
  VULKAN_API_PICK_PHYSICAL_DEVICE_ERROR,
  VULKAN_API_CREATE_LOGICAL_DEVICE_ERROR,
  VULKAN_API_CREATE_COMMAND_POOL_ERROR,
  VULKAN_API_GENERAL_FAILURE_ERROR,
  VULKAN_API_LAST_ERROR
};

uint_fast8_t api_vulkan_init(struct APICommon* api_common);
void api_vulkan_delete(struct APICommon* api_common);

// TODO: These have been spread to other files and should be split up
enum VULKAN_RENDERER_STATUS {
  VULKAN_RENDERER_SUCCESS = 0,
  VULKAN_RENDERER_NO_PRESENTABLE_DEVICE_ERROR,
  VULKAN_RENDERER_CREATE_WINDOW_ERROR,
  VULKAN_RENDERER_CREATE_INSTANCE_ERROR,
  VULKAN_RENDERER_SETUP_DEBUG_MESSENGER_ERROR,
  VULKAN_RENDERER_CREATE_SURFACE_ERROR,
  VULKAN_RENDERER_PICK_PHYSICAL_DEVICE_ERROR,
  VULKAN_RENDERER_CREATE_LOGICAL_DEVICE_ERROR,
  VULKAN_RENDERER_CREATE_SWAP_CHAIN_ERROR,
  VULKAN_RENDERER_CREATE_IMAGE_VIEWS_ERROR,
  VULKAN_RENDERER_CREATE_RENDER_PASS_ERROR,
  VULKAN_RENDERER_CREATE_GRAPHICS_PIPELINE_ERROR,
  VULKAN_RENDERER_CREATE_FRAME_BUFFER_ERROR,
  VULKAN_RENDERER_CREATE_COMMAND_POOL_ERROR,
  VULKAN_RENDERER_CREATE_COMMAND_BUFFER_ERROR,
  VULKAN_RENDERER_CREATE_SYNC_OBJECT_ERROR,
  VULKAN_RENDERER_LAST_ERROR
};

struct SamplerSettings {
  uint32_t mip_levels;

  VkFilter min_filter;
  VkFilter mag_filter;
  VkSamplerMipmapMode mipmap_mode;

  VkSamplerAddressMode address_mode;

  VkBool32 anisotropy_enable;
  float max_anisotropy;
};

VkSampleCountFlagBits vulkan_graphics_utils_get_max_msaa_samples(struct VulkanAPI* vulkan_api);
uint_fast8_t vulkan_graphics_utils_create_image_view(struct VkDevice_T* device, VkImage image, VkFormat format, VkImageAspectFlags aspect_flags, uint32_t mip_levels, VkImageView* image_view);
uint_fast8_t vulkan_graphics_utils_create_image(struct VkDevice_T* device, struct VkPhysicalDevice_T* physical_device, uint32_t width, uint32_t height, uint32_t mip_levels, VkSampleCountFlagBits num_samples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image, VkDeviceMemory* image_memory);
uint_fast8_t vulkan_graphics_utils_create_buffer(struct VkDevice_T* device, struct VkPhysicalDevice_T* physical_device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* buffer_memory);
uint_fast8_t vulkan_graphics_utils_transition_image_layout(struct VkDevice_T* device, struct VkQueue_T* graphics_queue, struct VkCommandPool_T* command_pool, VkImage image, VkImageLayout old_layout, VkImageLayout new_layout, uint32_t mip_levels);
uint32_t vulkan_graphics_utils_find_memory_type(struct VkPhysicalDevice_T* physical_device, uint32_t typeFilter, VkMemoryPropertyFlags properties);
VkCommandBuffer vulkan_graphics_utils_begin_single_time_commands(struct VkDevice_T* device, struct VkCommandPool_T* command_pool);
void vulkan_graphics_utils_end_single_time_commands(struct VkDevice_T* device, struct VkQueue_T* graphics_queue, struct VkCommandPool_T* command_pool, VkCommandBuffer command_buffer);
uint_fast8_t vulkan_graphics_utils_create_sampler(struct VkDevice_T* device, VkSampler* texture_sampler, struct SamplerSettings sampler_settings);
void vulkan_graphics_utils_copy_buffer_to_image(struct VkDevice_T* device, struct VkQueue_T* graphics_queue, struct VkCommandPool_T* command_pool, VkBuffer* buffer, VkImage* image, uint32_t width, uint32_t height);
void vulkan_graphics_utils_generate_mipmaps(struct VkDevice_T* device, VkPhysicalDevice physical_device, struct VkQueue_T* graphics_queue, struct VkCommandPool_T* command_pool, VkImage image, VkFormat format, uint32_t tex_width, uint32_t tex_height, uint32_t mip_levels);
VkFormat vulkan_graphics_utils_find_depth_format(VkPhysicalDevice physical_device);
void vulkan_graphics_utils_create_color_attachment(VkFormat image_format, struct VkAttachmentDescription* color_attachment);
void vulkan_graphics_utils_create_depth_attachment(VkPhysicalDevice physical_device, struct VkAttachmentDescription* depth_attachment);
void graphics_utils_copy_buffer(struct VulkanAPI* vulkan_api, VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size);
void graphics_utils_copy_buffer_offset(struct VulkanAPI* vulkan_api, VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size, uint32_t offset);
void vulkan_graphics_utils_setup_vertex_buffer(struct VulkanAPI* vulkan_api, struct Vector* vertices, VkBuffer* vertex_buffer, VkDeviceMemory* vertex_buffer_memory);
void vulkan_graphics_utils_setup_vertex_buffer_pool(struct VulkanAPI* vulkan_api, struct Vector* vertices, size_t total_pool_elements, VkBuffer* vertex_buffer, VkDeviceMemory* vertex_buffer_memory);
void vulkan_graphics_utils_update_vertex_buffer(struct VulkanAPI* vulkan_api, struct Vector* vertices, VkBuffer* vertex_buffer);
void vulkan_graphics_utils_setup_index_buffer(struct VulkanAPI* vulkan_api, struct Vector* indices, VkBuffer* index_buffer, VkDeviceMemory* index_buffer_memory);
void vulkan_graphics_utils_setup_index_buffer_pool(struct VulkanAPI* vulkan_api, struct Vector* indices, size_t total_pool_elements, VkBuffer* index_buffer, VkDeviceMemory* index_buffer_memory);
void vulkan_graphics_utils_update_index_buffer(struct VulkanAPI* vulkan_api, struct Vector* indices, VkBuffer* index_buffer);
void vulkan_graphics_utils_setup_uniform_buffer(struct VulkanAPI* vulkan_api, size_t memory_size, VkBuffer* uniform_buffer, VkDeviceMemory* uniform_buffer_memory);
uint_fast8_t vulkan_graphics_utils_setup_descriptor(struct VulkanAPI* vulkan_api, struct VkDescriptorSetLayout_T* descriptor_set_layout, struct VkDescriptorPool_T* descriptor_pool, VkDescriptorSet* descriptor_set);
VkDescriptorBufferInfo vulkan_graphics_utils_setup_descriptor_buffer_info(size_t memory_size, VkBuffer* uniform_buffer);
void vulkan_graphics_utils_setup_descriptor_buffer(VkWriteDescriptorSet* dcs, size_t index, VkDescriptorSet* descriptor_set, VkDescriptorBufferInfo* buffer_info);
VkDescriptorImageInfo vulkan_graphics_utils_setup_descriptor_image_info(VkImageView* texture_image_view, VkSampler* texture_sampler);
void vulkan_graphics_utils_setup_descriptor_image(VkWriteDescriptorSet* dcs, size_t index, VkDescriptorSet* descriptor_set, VkDescriptorImageInfo* image_info);
VkVertexInputBindingDescription vulkan_graphics_utils_get_binding_description(uint32_t memory_size);
uint_fast8_t vulkan_graphics_utils_create_descriptors(struct VulkanAPI* vulkan_api, VkDescriptorSet* descriptor_set, VkDescriptorSetLayout* descriptor_set_layout, VkDescriptorPool* descriptor_pool, uint_fast32_t descriptors);
#endif
