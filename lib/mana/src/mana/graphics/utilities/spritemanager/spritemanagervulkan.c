#include "mana/graphics/utilities/spritemanager/spritemanagervulkan.h"

uint_fast8_t sprite_manager_vulkan_init_sprite_pool(struct SpriteManagerCommon* sprite_manager_common, struct APICommon* api_common, uint32_t width, uint32_t height, uint_fast8_t supersample_scale, struct GBufferCommon* gbuffer_common, uint_fast8_t msaa_samples, uint_fast32_t descriptors) {
  sprite_manager_common->sprite_manager_vulkan.sprite_descriptor_set = calloc(descriptors, sizeof(VkDescriptorSet));
  return vulkan_graphics_utils_create_descriptors(&(api_common->vulkan_api), sprite_manager_common->sprite_manager_vulkan.sprite_descriptor_set, &(sprite_manager_common->sprite_shader.shader.shader_common.shader_vulkan.descriptor_set_layout), &(sprite_manager_common->sprite_shader.shader.shader_common.shader_vulkan.descriptor_pool), sprite_manager_common->sprite_shader.shader.shader_common.shader_settings.descriptors);
}

void sprite_manager_vulkan_delete_sprite(struct SpriteManagerCommon* sprite_manager_common, struct APICommon* api_common) {
  free(sprite_manager_common->sprite_manager_vulkan.sprite_descriptor_set);
}

void sprite_manager_vulkan_add_sprite(struct SpriteManagerCommon* sprite_manager_common, struct APICommon* api_common, struct Sprite* sprite, size_t sprite_num) {
  sprite->sprite_common.sprite_vulkan.descriptor_set = &(sprite_manager_common->sprite_manager_vulkan.sprite_descriptor_set[sprite_num]);
}
