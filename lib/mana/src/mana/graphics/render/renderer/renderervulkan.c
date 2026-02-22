#include "mana/graphics/render/renderer/renderervulkan.h"

static bool window_device_can_present(struct APICommon *api_common, struct SwapChain *swap_chain) {
  struct VulkanAPI *vulkan_api = &api_common->vulkan_api;

  uint32_t queue_family_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(vulkan_api->physical_device, &queue_family_count, NULL);
  VkQueueFamilyProperties *queue_families = alloca(queue_family_count * sizeof(VkQueueFamilyProperties));
  vkGetPhysicalDeviceQueueFamilyProperties(vulkan_api->physical_device, &queue_family_count, queue_families);

  for (uint32_t queue_family_num = 0; queue_family_num < queue_family_count; queue_family_num++) {
    VkBool32 present_support = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(vulkan_api->physical_device, queue_family_num, swap_chain->swap_chain_common.swap_chain_vulkan.surface, &present_support);

    if (queue_families[queue_family_num].queueCount > 0 && present_support) {
      (&vulkan_api->indices)->present_family = queue_family_num;
      return true;
    }
  }

  return false;
}

uint8_t vulkan_renderer_init(struct APICommon *api_common, struct Surface *surface, struct SwapChain *swap_chain, struct GBuffer *gbuffer, struct PostProcess *post_process, struct RendererSettings *renderer_settings) {
  /* create the window surface */
  VkWin32SurfaceCreateInfoKHR surface_create_info = {0};
  surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  surface_create_info.pNext = NULL;
  surface_create_info.flags = 0;
  surface_create_info.hinstance = surface->hinstance;
  surface_create_info.hwnd = surface->hwnd;
  VkResult result = vkCreateWin32SurfaceKHR(api_common->vulkan_api.instance, &surface_create_info, NULL, &(swap_chain->swap_chain_common.swap_chain_vulkan.surface));
  if (result != VK_SUCCESS) {
    log_message(LOG_SEVERITY_ERROR, "Error creating window surface!\n");
    return 1;
  }

  // TODO: If device cannot render, check for new device that can then recreate core?
  if (!window_device_can_present(api_common, swap_chain)) {
    log_message(LOG_SEVERITY_ERROR, "Error graphics device cannot present to surface!\n");
    return VULKAN_RENDERER_NO_PRESENTABLE_DEVICE_ERROR;
  }

  return 0;
}

void vulkan_renderer_delete(struct APICommon *api_common, struct Surface *surface, struct SwapChain *swap_chain, struct GBuffer *gbuffer, struct PostProcess *post_process, struct RendererSettings *renderer_settings) {
  vkDestroySurfaceKHR(api_common->vulkan_api.instance, swap_chain->swap_chain_common.swap_chain_vulkan.surface, NULL);
}

void vulkan_renderer_wait_for_device(struct APICommon *api_common) {
  vkDeviceWaitIdle(api_common->vulkan_api.device);
}
