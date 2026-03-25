#include "mana/graphics/apis/apivulkan.h"

static uint_fast8_t api_vulkan_create_instance(struct VulkanAPI* vulkan_api);
static uint_fast8_t api_vulkan_setup_debug_messenger(struct VulkanAPI* vulkan_api);
static uint_fast8_t api_vulkan_pick_physical_device(struct VulkanAPI* vulkan_api);
static bool api_vulkan_device_can_render(struct VulkanAPI* vulkan_api, VkPhysicalDevice device);
static uint_fast8_t api_vulkan_create_logical_device(struct VulkanAPI* vulkan_api);
static uint_fast8_t api_vulkan_create_command_pool(struct VulkanAPI* vulkan_api);
static bool api_vulkan_check_validation_layer_support(void);
static void vulkan_command_pool_cleanup(struct VulkanAPI* vulkan_api);
static void vulkan_device_cleanup(struct VulkanAPI* vulkan_api);
static void vulkan_debug_cleanup(struct VulkanAPI* vulkan_api);

static inline uint_fast8_t api_vulkan_init_wrapper(struct APICommon* api_common) {
  struct VulkanAPI* vulkan_api = &(api_common->vulkan_api);

  vulkan_api->physical_device = VK_NULL_HANDLE;

  uint_fast8_t vulkan_error_code;
  if ((vulkan_error_code = api_vulkan_create_instance(vulkan_api)) != VULKAN_API_SUCCESS)
    goto vulkan_api_create_instance_cleanup;
  if ((vulkan_error_code = api_vulkan_setup_debug_messenger(vulkan_api)) != VULKAN_API_SUCCESS)
    goto vulkan_debug_error;
  if ((vulkan_error_code = api_vulkan_pick_physical_device(vulkan_api)) != VULKAN_API_SUCCESS)
    goto vulkan_pick_device_error;
  if ((vulkan_error_code = api_vulkan_create_logical_device(vulkan_api)) != VULKAN_API_SUCCESS)
    goto vulkan_device_error;
  if ((vulkan_error_code = api_vulkan_create_command_pool(vulkan_api)) != VULKAN_API_SUCCESS)
    goto vulkan_command_pool_error;

  return vulkan_error_code;

vulkan_command_pool_error:
  vulkan_command_pool_cleanup(vulkan_api);
vulkan_device_error:
  vulkan_device_cleanup(vulkan_api);
vulkan_pick_device_error:
vulkan_debug_error:
  vulkan_debug_cleanup(vulkan_api);
vulkan_api_create_instance_cleanup:

  return vulkan_error_code;
}

uint_fast8_t api_vulkan_init(struct APICommon* api_common) {
  const uint_fast8_t api_vulkan_error = api_vulkan_init_wrapper(api_common);
  switch (api_vulkan_error) {
    case (VULKAN_API_SUCCESS): {
      break;
    }
    case (VULKAN_API_CREATE_INSTANCE_ERROR): {
      log_message(LOG_SEVERITY_ERROR, "Failed to create a Vulkan state instance!\n");
      return API_ERROR;
    }
    case (VULKAN_API_SETUP_DEBUG_MESSENGER_ERROR): {
      log_message(LOG_SEVERITY_ERROR, "Failed to set up Vulkan debug state messenger!\n");
      return API_ERROR;
    }
    case (VULKAN_API_PICK_PHYSICAL_DEVICE_ERROR): {
      log_message(LOG_SEVERITY_ERROR, "Failed to find a suitable GPU for Vulkan state!\n");
      return API_ERROR;
    }
    case (VULKAN_API_CREATE_LOGICAL_DEVICE_ERROR): {
      log_message(LOG_SEVERITY_ERROR, "Failed to create a Vulkan state logical device!\n");
      return API_ERROR;
    }
    case (VULKAN_API_CREATE_COMMAND_POOL_ERROR): {
      log_message(LOG_SEVERITY_ERROR, "Failed to create Vulkan state command pool!\n");
      return API_ERROR;
    }
    default: {
      log_message(LOG_SEVERITY_CRITICAL, "Unknown Vulkan state error! Error code: %d\n", api_vulkan_error);
      return API_ERROR;
    }
  }
  return api_vulkan_error;
}

void api_vulkan_delete(struct APICommon* api_common) {
  struct VulkanAPI* vulkan_api = &(api_common->vulkan_api);

  vulkan_command_pool_cleanup(vulkan_api);
  vulkan_device_cleanup(vulkan_api);
  vulkan_debug_cleanup(vulkan_api);
}

static void vulkan_device_cleanup(struct VulkanAPI* vulkan_api) {
  vkDestroyDevice(vulkan_api->device, NULL);
}

static void vulkan_command_pool_cleanup(struct VulkanAPI* vulkan_api) {
  vkDestroyCommandPool(vulkan_api->device, vulkan_api->command_pool, NULL);
}

static void vulkan_debug_cleanup(struct VulkanAPI* vulkan_api) {
#ifdef ENABLE_VALIDATION_LAYERS
  void* value = (void*)vkGetInstanceProcAddr(vulkan_api->instance, "vkDestroyDebugUtilsMessengerEXT");
  PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)value;
  if (func != NULL)
    func(vulkan_api->instance, vulkan_api->debug_messenger, NULL);
#endif
}

static bool api_vulkan_check_surface_extension_support(const char** graphics_lbrary_extensions, uint32_t* graphics_library_extension_count) {
  // Enumerate the available instance extensions
  if (graphics_lbrary_extensions != NULL) {
    graphics_lbrary_extensions[0] = VK_KHR_SURFACE_EXTENSION_NAME;
    graphics_lbrary_extensions[1] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
    return true;
  }

  uint32_t extension_count = 0;
  vkEnumerateInstanceExtensionProperties(NULL, &extension_count, NULL);

  VkExtensionProperties* extensions = (VkExtensionProperties*)alloca(extension_count * sizeof(VkExtensionProperties));
  vkEnumerateInstanceExtensionProperties(NULL, &extension_count, extensions);

  // Check if the VK_KHR_surface and VK_KHR_win32_surface extensions are available
  bool surface_extension_found = false;
  bool win32_extension_found = false;
  for (uint32_t i = 0; i < extension_count; i++) {
    if (strcmp(extensions[i].extensionName, VK_KHR_SURFACE_EXTENSION_NAME) == 0)
      surface_extension_found = true;
    if (strcmp(extensions[i].extensionName, VK_KHR_WIN32_SURFACE_EXTENSION_NAME) == 0)
      win32_extension_found = true;
    if (surface_extension_found && win32_extension_found)
      break;
  }

  if (!surface_extension_found || !win32_extension_found) {
    log_message(LOG_SEVERITY_ERROR, "VK_KHR_surface and VK_KHR_win32_surface extensions are not both available.\n");
    return false;
  }

  if (surface_extension_found && win32_extension_found && graphics_lbrary_extensions == NULL)
    *graphics_library_extension_count = 2;

  return true;
}

static uint_fast8_t api_vulkan_create_instance(struct VulkanAPI* vulkan_api) {
#ifdef ENABLE_VALIDATION_LAYERS
  if (!api_vulkan_check_validation_layer_support())
    return VULKAN_API_CREATE_INSTANCE_ERROR;
#endif

  VkApplicationInfo app_info;
  memset(&app_info, 0, sizeof(app_info));
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  // TODO: Pull name and version from engine
  app_info.pApplicationName = "TODO";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "Mana";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo create_info;
  memset(&create_info, 0, sizeof(create_info));
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;

  uint32_t graphics_library_extension_count = 0;
  api_vulkan_check_surface_extension_support(NULL, &graphics_library_extension_count);

#ifdef ENABLE_VALIDATION_LAYERS
  graphics_library_extension_count++;
#endif

  const char** graphics_lbrary_extensions = (const char**)alloca(graphics_library_extension_count * sizeof(char*));
  if (!api_vulkan_check_surface_extension_support(graphics_lbrary_extensions, &graphics_library_extension_count))
    return VULKAN_API_CREATE_INSTANCE_ERROR;

  create_info.enabledExtensionCount = graphics_library_extension_count;
  create_info.ppEnabledExtensionNames = graphics_lbrary_extensions;

#ifdef ENABLE_VALIDATION_LAYERS
  graphics_lbrary_extensions[graphics_library_extension_count - 1] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
  create_info.enabledLayerCount = (uint32_t)1;
  create_info.ppEnabledLayerNames = validation_layers;
#else
  create_info.enabledLayerCount = 0;
#endif

  VkResult vulkan_instance_status = vkCreateInstance(&create_info, NULL, &vulkan_api->instance);

  if (vulkan_instance_status == VK_ERROR_INCOMPATIBLE_DRIVER)
    log_message(LOG_SEVERITY_ERROR, "GPU driver is incompatible with vulkan!\n");

  if (vulkan_instance_status != VK_SUCCESS)
    return VULKAN_API_CREATE_INSTANCE_ERROR;

  return VULKAN_API_SUCCESS;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data, void* p_user_data) {
  char type[32] = {0};
  switch (message_type) {
    case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT: {
      strcpy_s(type, sizeof(type), "general");
      break;
    }
    case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT: {
      strcpy_s(type, sizeof(type), "validation");
      break;
    }
    case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT: {
      strcpy_s(type, sizeof(type), "performance");
      break;
    }
    case VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT: {
      strcpy_s(type, sizeof(type), "device address binding");
      break;
    }
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcovered-switch-default"
    default: {
      __builtin_unreachable();
    }
#pragma clang diagnostic pop
  }

  switch (message_severity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: {
      log_message(LOG_SEVERITY_DEBUG, "Validation Layer[%s]: %s\n", type, p_callback_data->pMessage);
      break;
    }
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: {
      log_message(LOG_SEVERITY_DEBUG, "Validation Layer[%s]: %s\n", type, p_callback_data->pMessage);
      break;
    }
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: {
      log_message(LOG_SEVERITY_WARNING, "Validation Layer[%s]: %s\n", type, p_callback_data->pMessage);
      break;
    }
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: {
      log_message(LOG_SEVERITY_ERROR, "Validation Layer[%s]: %s\n", type, p_callback_data->pMessage);
      break;
    }
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT: {
      log_message(LOG_SEVERITY_CRITICAL, "Validation Layer[%s]: %s\n", type, p_callback_data->pMessage);
      break;
    }
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcovered-switch-default"
    default: {
      __builtin_unreachable();
    }
#pragma clang diagnostic pop
  }

  return VK_FALSE;
}

static VkResult create_debug_utils_messenger_ext(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* p_create_info, const VkAllocationCallbacks* p_allocator, VkDebugUtilsMessengerEXT* p_debug_messenger) {
  void* value = (void*)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
  PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)value;
  if (func != NULL)
    return func(instance, p_create_info, p_allocator, p_debug_messenger);
  else
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

static uint_fast8_t api_vulkan_setup_debug_messenger(struct VulkanAPI* vulkan_api) {
#ifdef ENABLE_VALIDATION_LAYERS
  if (!api_vulkan_check_validation_layer_support())
    return VULKAN_API_SETUP_DEBUG_MESSENGER_ERROR;

  VkDebugUtilsMessengerCreateInfoEXT debug_info;
  memset(&debug_info, 0, sizeof(debug_info));
  debug_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  debug_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  debug_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  debug_info.pfnUserCallback = debug_callback;

  if (create_debug_utils_messenger_ext(vulkan_api->instance, &debug_info, NULL, &vulkan_api->debug_messenger) != VK_SUCCESS)
    return VULKAN_API_SETUP_DEBUG_MESSENGER_ERROR;
#endif

  return VULKAN_API_SUCCESS;
}

static bool api_vulkan_check_swap_chain_support(VkPhysicalDevice device) {
  // Enumerate available device extensions
  uint32_t extension_count = 0;
  vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, NULL);
  VkExtensionProperties* extensions = (VkExtensionProperties*)alloca(extension_count * sizeof(VkExtensionProperties));
  vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, extensions);

  // Check if VK_KHR_SWAPCHAIN_EXTENSION_NAME is available
  VkBool32 swap_chain_extension_found = VK_FALSE;
  for (uint32_t i = 0; i < extension_count; i++) {
    if (strcmp(extensions[i].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
      swap_chain_extension_found = VK_TRUE;
      break;
    }
  }

  if (swap_chain_extension_found)
    return true;

  log_message(LOG_SEVERITY_ERROR, "Swap chain extension not found!\n");
  return false;
}

static uint_fast8_t api_vulkan_pick_physical_device(struct VulkanAPI* vulkan_api) {
  uint32_t device_count = 0;
  vkEnumeratePhysicalDevices(vulkan_api->instance, &device_count, NULL);

  if (device_count == 0)
    return VULKAN_API_PICK_PHYSICAL_DEVICE_ERROR;

  VkPhysicalDevice* devices = (VkPhysicalDevice*)alloca(device_count * sizeof(VkPhysicalDevice));
  vkEnumeratePhysicalDevices(vulkan_api->instance, &device_count, devices);

  VkPhysicalDeviceProperties current_device_properties = {0};
  uint64_t current_largest_heap = 0;
  bool discrete_selected = false;

  for (uint32_t device_num = 0; device_num < device_count; device_num++) {
    VkPhysicalDevice device = devices[device_num];
    VkPhysicalDeviceProperties device_properties = {0};
    vkGetPhysicalDeviceProperties(device, &device_properties);

    if (api_vulkan_device_can_render(vulkan_api, device)) {
      if (!api_vulkan_check_swap_chain_support(device))
        return VULKAN_API_PICK_PHYSICAL_DEVICE_ERROR;

      if (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        VkPhysicalDeviceMemoryProperties device_memory_properties = {0};
        vkGetPhysicalDeviceMemoryProperties(device, &device_memory_properties);

        for (uint32_t heap_num = 0; heap_num < device_memory_properties.memoryHeapCount; heap_num++) {
          VkMemoryHeap device_heap = device_memory_properties.memoryHeaps[heap_num];

          if (device_heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT && device_heap.size > current_largest_heap) {
            discrete_selected = true;
            current_largest_heap = device_heap.size;
            current_device_properties = device_properties;
            vulkan_api->physical_device = device;
          }
        }
      } else if (current_device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
        if (discrete_selected == false) {
          current_device_properties = device_properties;
          vulkan_api->physical_device = device;
        }
      } else
        log_message(LOG_SEVERITY_INFO, "Unknown device: %s\n", device_properties.deviceName);
    }
  }

  log_message(LOG_SEVERITY_DEBUG, "Selected device: %s\n", current_device_properties.deviceName);

  if (vulkan_api->physical_device == VK_NULL_HANDLE)
    return VULKAN_API_PICK_PHYSICAL_DEVICE_ERROR;

  return VULKAN_API_SUCCESS;
}

static bool api_vulkan_device_can_render(struct VulkanAPI* vulkan_api, VkPhysicalDevice device) {
  uint32_t queue_family_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, NULL);
  VkQueueFamilyProperties* queue_families = (VkQueueFamilyProperties*)alloca(queue_family_count * sizeof(VkQueueFamilyProperties));
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);

  bool graphics_family_found = false;
  for (uint32_t queue_family_num = 0; queue_family_num < queue_family_count; queue_family_num++) {
    if (queue_families[queue_family_num].queueCount > 0 && queue_families[queue_family_num].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      (&vulkan_api->indices)->graphics_family = queue_family_num;
      graphics_family_found = true;
      break;
    }
  }

  if (!graphics_family_found)
    return false;

  return true;
}

static uint_fast8_t api_vulkan_create_logical_device(struct VulkanAPI* vulkan_api) {
  const uint32_t unique_queue_families[2] = {vulkan_api->indices.graphics_family, vulkan_api->indices.present_family};
  const int_fast32_t unique_queue_family_count = (unique_queue_families[0] == unique_queue_families[1]) ? 1 : 2;

  VkDeviceQueueCreateInfo queue_create_infos[2];
  memset(queue_create_infos, 0, sizeof(queue_create_infos));
  float queue_priority = 1.0f;
  for (int_fast32_t queue_num = 0; queue_num < unique_queue_family_count; queue_num++) {
    VkDeviceQueueCreateInfo queue_create_info;
    memset(&queue_create_info, 0, sizeof(queue_create_info));
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = (uint32_t)queue_num;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &queue_priority;
    queue_create_infos[queue_num] = queue_create_info;
  }

  struct VkPhysicalDeviceFeatures device_features = {0};
  device_features.samplerAnisotropy = VK_TRUE;

  struct VkDeviceCreateInfo device_info;
  memset(&device_info, 0, sizeof(device_info));
  device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

  device_info.queueCreateInfoCount = (uint32_t)unique_queue_family_count;
  device_info.pQueueCreateInfos = queue_create_infos;

  device_info.pEnabledFeatures = &device_features;

  device_info.enabledExtensionCount = (uint32_t)VULKAN_DEVICE_EXTENSION_COUNT;
  device_info.ppEnabledExtensionNames = device_extensions;

#ifdef ENABLE_VALIDATION_LAYERS
  device_info.enabledLayerCount = (uint32_t)VULKAN_VALIDATION_LAYER_COUNT;
  device_info.ppEnabledLayerNames = validation_layers;
#else
  device_info.enabledLayerCount = 0;
#endif

  if (vkCreateDevice(vulkan_api->physical_device, &device_info, NULL, &vulkan_api->device) != VK_SUCCESS)
    return VULKAN_API_CREATE_LOGICAL_DEVICE_ERROR;

  vkGetDeviceQueue(vulkan_api->device, vulkan_api->indices.graphics_family, 0, &vulkan_api->graphics_queue);
  vkGetDeviceQueue(vulkan_api->device, vulkan_api->indices.present_family, 0, &vulkan_api->present_queue);

  return VULKAN_API_SUCCESS;
}

static uint_fast8_t api_vulkan_create_command_pool(struct VulkanAPI* vulkan_api) {
  VkCommandPoolCreateFlags command_pool_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  VkCommandPoolCreateInfo pool_info;
  memset(&pool_info, 0, sizeof(pool_info));
  pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pool_info.queueFamilyIndex = vulkan_api->indices.graphics_family;
  pool_info.flags = command_pool_flags;

  if (vkCreateCommandPool(vulkan_api->device, &pool_info, NULL, &vulkan_api->command_pool) != VK_SUCCESS)
    return VULKAN_API_CREATE_COMMAND_POOL_ERROR;

  return VULKAN_API_SUCCESS;
}

static bool api_vulkan_check_validation_layer_support(void) {
  uint32_t layer_count;
  vkEnumerateInstanceLayerProperties(&layer_count, NULL);

  VkLayerProperties* available_layers = (VkLayerProperties*)calloc(layer_count, sizeof(VkLayerProperties));
  vkEnumerateInstanceLayerProperties(&layer_count, available_layers);

  for (uint32_t layer_name_num = 0; layer_name_num < VULKAN_VALIDATION_LAYER_COUNT; layer_name_num++) {
    bool layer_found = false;

    for (uint32_t layer_property_num = 0; layer_property_num < layer_count; layer_property_num++) {
      if (strcmp(validation_layers[layer_name_num], available_layers[layer_property_num].layerName) == 0) {
        layer_found = true;
        break;
      }
    }

    if (layer_found == false)
      return false;
  }

  free(available_layers);

  return true;
}

uint_fast8_t vulkan_graphics_utils_create_image_view(struct VkDevice_T* device, VkImage image, VkFormat format, VkImageAspectFlags aspect_flags, uint32_t mip_levels, uint32_t layer_count, bool is_array, VkImageView* image_view) {
  VkImageViewCreateInfo view_info;
  memset(&view_info, 0, sizeof(view_info));
  view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  view_info.image = image;
  view_info.viewType = is_array ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
  view_info.format = format;

  view_info.subresourceRange.aspectMask = aspect_flags;
  view_info.subresourceRange.baseMipLevel = 0;
  view_info.subresourceRange.levelCount = mip_levels;
  view_info.subresourceRange.baseArrayLayer = 0;
  view_info.subresourceRange.layerCount = layer_count;

  if (vkCreateImageView(device, &view_info, NULL, image_view) != VK_SUCCESS) {
    log_message(LOG_SEVERITY_ERROR, "failed to create texture image view!\n");
    return 1;
  }

  return VULKAN_API_SUCCESS;
}

uint_fast8_t vulkan_graphics_utils_create_image(struct VkDevice_T* device, struct VkPhysicalDevice_T* physical_device, uint32_t width, uint32_t height, uint32_t mip_levels, uint32_t layer_count, VkSampleCountFlagBits num_samples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image, VkDeviceMemory* image_memory) {
  VkImageCreateInfo image_info;
  memset(&image_info, 0, sizeof(image_info));
  image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  image_info.imageType = VK_IMAGE_TYPE_2D;
  image_info.extent.width = width;
  image_info.extent.height = height;
  image_info.extent.depth = 1;
  image_info.mipLevels = mip_levels;
  image_info.arrayLayers = layer_count;
  image_info.format = format;
  image_info.tiling = tiling;
  image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  image_info.usage = usage;
  image_info.samples = num_samples;
  image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateImage(device, &image_info, NULL, image) != VK_SUCCESS) {
    log_message(LOG_SEVERITY_ERROR, "failed to create image!\n");
    return 1;
  }

  VkMemoryRequirements mem_mequirements;
  vkGetImageMemoryRequirements(device, *image, &mem_mequirements);

  VkMemoryAllocateInfo alloc_info;
  memset(&alloc_info, 0, sizeof(alloc_info));
  alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.allocationSize = mem_mequirements.size;
  alloc_info.memoryTypeIndex = vulkan_graphics_utils_find_memory_type(physical_device, mem_mequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(device, &alloc_info, NULL, image_memory) != VK_SUCCESS) {
    log_message(LOG_SEVERITY_ERROR, "failed to allocate image memory!\n");
    return 2;
  }

  vkBindImageMemory(device, *image, *image_memory, 0);

  return VULKAN_API_SUCCESS;
}

uint_fast8_t vulkan_graphics_utils_create_buffer(struct VkDevice_T* device, struct VkPhysicalDevice_T* physical_device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* buffer_memory) {
  VkBufferCreateInfo buffer_info;
  memset(&buffer_info, 0, sizeof(buffer_info));
  buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_info.size = size;
  buffer_info.usage = usage;
  buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(device, &buffer_info, NULL, buffer) != VK_SUCCESS) {
    log_message(LOG_SEVERITY_ERROR, "failed to create buffer!\n");
    return 1;
  }

  VkMemoryRequirements mem_requirements;
  vkGetBufferMemoryRequirements(device, *buffer, &mem_requirements);

  VkMemoryAllocateInfo alloc_info;
  memset(&alloc_info, 0, sizeof(alloc_info));
  alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.allocationSize = mem_requirements.size;
  alloc_info.memoryTypeIndex = vulkan_graphics_utils_find_memory_type(physical_device, mem_requirements.memoryTypeBits, properties);

  if (vkAllocateMemory(device, &alloc_info, NULL, buffer_memory) != VK_SUCCESS) {
    log_message(LOG_SEVERITY_ERROR, "failed to allocate buffer memory!\n");
    return 1;
  }

  vkBindBufferMemory(device, *buffer, *buffer_memory, 0);

  return VULKAN_API_SUCCESS;
}

uint_fast8_t vulkan_graphics_utils_transition_image_layout(struct VkDevice_T* device, struct VkQueue_T* graphics_queue, struct VkCommandPool_T* command_pool, VkImage image, VkImageLayout old_layout, VkImageLayout new_layout, uint32_t mip_levels, uint32_t layer_count) {
  VkCommandBuffer commandBuffer = vulkan_graphics_utils_begin_single_time_commands(device, command_pool);

  VkImageMemoryBarrier barrier;
  memset(&barrier, 0, sizeof(barrier));
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = old_layout;
  barrier.newLayout = new_layout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = image;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = mip_levels;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = layer_count;

  VkPipelineStageFlags source_stage = {0};
  VkPipelineStageFlags destination_stage = {0};

  if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else {
    log_message(LOG_SEVERITY_ERROR, "unsupported layout transition!");
    return 1;
  }

  vkCmdPipelineBarrier(commandBuffer, source_stage, destination_stage, 0, 0, NULL, 0, NULL, 1, &barrier);

  vulkan_graphics_utils_end_single_time_commands(device, graphics_queue, command_pool, commandBuffer);

  return VULKAN_API_SUCCESS;
}

uint32_t vulkan_graphics_utils_find_memory_type(struct VkPhysicalDevice_T* physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties mem_properties;
  vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);

  for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++)
    if ((type_filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties)
      return i;

  printf("failed to find suitable memory type!\n");

  return VULKAN_API_SUCCESS;
}

VkCommandBuffer vulkan_graphics_utils_begin_single_time_commands(struct VkDevice_T* device, struct VkCommandPool_T* command_pool) {
  VkCommandBufferAllocateInfo allocInfo;
  memset(&allocInfo, 0, sizeof(allocInfo));
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = command_pool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer command_buffer;
  vkAllocateCommandBuffers(device, &allocInfo, &command_buffer);

  VkCommandBufferBeginInfo begin_info;
  memset(&begin_info, 0, sizeof(begin_info));
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(command_buffer, &begin_info);

  return command_buffer;
}

void vulkan_graphics_utils_end_single_time_commands(struct VkDevice_T* device, struct VkQueue_T* graphics_queue, struct VkCommandPool_T* command_pool, VkCommandBuffer command_buffer) {
  vkEndCommandBuffer(command_buffer);

  VkSubmitInfo submit_info;
  memset(&submit_info, 0, sizeof(submit_info));
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &command_buffer;

  vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
  vkQueueWaitIdle(graphics_queue);

  vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);
}

uint_fast8_t vulkan_graphics_utils_create_sampler(struct VkDevice_T* device, VkSampler* texture_sampler, struct SamplerSettings sampler_settings) {
  VkSamplerCreateInfo sampler_info;
  memset(&sampler_info, 0, sizeof(sampler_info));
  sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

  sampler_info.magFilter = sampler_settings.mag_filter;
  sampler_info.minFilter = sampler_settings.min_filter;
  sampler_info.mipmapMode = sampler_settings.mipmap_mode;

  sampler_info.addressModeU = sampler_settings.address_mode;
  sampler_info.addressModeV = sampler_settings.address_mode;
  sampler_info.addressModeW = sampler_settings.address_mode;

  sampler_info.anisotropyEnable = sampler_settings.anisotropy_enable;
  sampler_info.maxAnisotropy = sampler_settings.anisotropy_enable ? sampler_settings.max_anisotropy : 1.0f;

  sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  sampler_info.unnormalizedCoordinates = VK_FALSE;
  sampler_info.compareEnable = VK_FALSE;
  sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;

  sampler_info.minLod = 0.0f;
  sampler_info.maxLod = (float)(sampler_settings.mip_levels - 1);
  sampler_info.mipLodBias = 0.0f;

  if (vkCreateSampler(device, &sampler_info, NULL, texture_sampler) != VK_SUCCESS) {
    log_message(LOG_SEVERITY_ERROR, "failed to create texture sampler!\n");
    return 1;
  }

  return VULKAN_API_SUCCESS;
}

void vulkan_graphics_utils_copy_buffer_to_image(VkDevice device, VkQueue graphics_queue, VkCommandPool command_pool, VkBuffer* buffer, VkImage* image, uint32_t width, uint32_t height, uint32_t layer_index, VkDeviceSize buffer_offset) {
  VkCommandBuffer command_buffer = vulkan_graphics_utils_begin_single_time_commands(device, command_pool);

  VkBufferImageCopy region = {0};
  region.bufferOffset = buffer_offset;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;
  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = layer_index;
  region.imageSubresource.layerCount = 1;
  region.imageOffset = (VkOffset3D){0, 0, 0};
  region.imageExtent = (VkExtent3D){width, height, 1};

  vkCmdCopyBufferToImage(command_buffer, *buffer, *image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

  vulkan_graphics_utils_end_single_time_commands(device, graphics_queue, command_pool, command_buffer);
}

void vulkan_graphics_utils_generate_mipmaps(struct VkDevice_T* device, VkPhysicalDevice physical_device, struct VkQueue_T* graphics_queue, struct VkCommandPool_T* command_pool, VkImage image, VkFormat format, uint32_t tex_width, uint32_t tex_height, uint32_t mip_levels) {
  VkFormatProperties format_properties;
  vkGetPhysicalDeviceFormatProperties(physical_device, format, &format_properties);

  if (!(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
    return;
  // throw std::runtime_error("texture image format does not support linear blitting!");

  VkCommandBuffer command_buffer = vulkan_graphics_utils_begin_single_time_commands(device, command_pool);

  VkImageMemoryBarrier barrier;
  memset(&barrier, 0, sizeof(barrier));
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.image = image;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;
  barrier.subresourceRange.levelCount = 1;

  uint32_t mip_width = tex_width;
  uint32_t mip_height = tex_height;

  for (uint32_t i = 1; i < mip_levels; i++) {
    barrier.subresourceRange.baseMipLevel = i - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

    vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

    VkImageBlit blit = {0};
    blit.srcOffsets[0] = (VkOffset3D){0, 0, 0};
    blit.srcOffsets[1] = (VkOffset3D){(int32_t)mip_width, (int32_t)mip_height, 1};
    blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit.srcSubresource.mipLevel = i - 1;
    blit.srcSubresource.baseArrayLayer = 0;
    blit.srcSubresource.layerCount = 1;
    blit.dstOffsets[0] = (VkOffset3D){0, 0, 0};
    blit.dstOffsets[1] = (VkOffset3D){mip_width > 1 ? mip_width / 2 : 1, mip_height > 1 ? mip_height / 2 : 1, 1};
    blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit.dstSubresource.mipLevel = i;
    blit.dstSubresource.baseArrayLayer = 0;
    blit.dstSubresource.layerCount = 1;

    vkCmdBlitImage(command_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

    if (mip_width > 1)
      mip_width /= 2;
    if (mip_height > 1)
      mip_height /= 2;
  }

  barrier.subresourceRange.baseMipLevel = mip_levels - 1;
  barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

  vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);
  vulkan_graphics_utils_end_single_time_commands(device, graphics_queue, command_pool, command_buffer);
}

#define TOTAL_CANDIDIATES 3
VkFormat vulkan_graphics_utils_find_depth_format(VkPhysicalDevice physical_device) {
  VkFormat candidate[TOTAL_CANDIDIATES] = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};
  VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
  VkFormatFeatureFlags features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

  for (uint_fast8_t loop_num = 0; loop_num < TOTAL_CANDIDIATES; loop_num++) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(physical_device, candidate[loop_num], &props);

    if (tiling == VK_IMAGE_TILING_LINEAR &&
        (props.linearTilingFeatures & features) == features) {
      return candidate[loop_num];
    } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
      return candidate[loop_num];
    }
  }

  return VK_FORMAT_UNDEFINED;
}

void vulkan_graphics_utils_create_color_attachment(VkFormat image_format, struct VkAttachmentDescription* color_attachment) {
  color_attachment->format = image_format;
  color_attachment->samples = VK_SAMPLE_COUNT_1_BIT;
  color_attachment->loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color_attachment->storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  color_attachment->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color_attachment->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  color_attachment->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  color_attachment->finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
}

void vulkan_graphics_utils_create_depth_attachment(VkPhysicalDevice physical_device, struct VkAttachmentDescription* depth_attachment) {
  depth_attachment->format = vulkan_graphics_utils_find_depth_format(physical_device);
  depth_attachment->samples = VK_SAMPLE_COUNT_1_BIT;
  depth_attachment->loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depth_attachment->storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depth_attachment->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depth_attachment->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depth_attachment->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depth_attachment->finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
}

void graphics_utils_copy_buffer(struct VulkanAPI* vulkan_api, VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size) {
  VkCommandBuffer command_buffer = vulkan_graphics_utils_begin_single_time_commands(vulkan_api->device, vulkan_api->command_pool);

  VkBufferCopy copy_region = {0};
  copy_region.size = size;
  vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_region);

  vulkan_graphics_utils_end_single_time_commands(vulkan_api->device, vulkan_api->graphics_queue, vulkan_api->command_pool, command_buffer);
}

void graphics_utils_copy_buffer_offset(struct VulkanAPI* vulkan_api, VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size, uint32_t offset) {
  VkCommandBuffer command_buffer = vulkan_graphics_utils_begin_single_time_commands(vulkan_api->device, vulkan_api->command_pool);

  VkBufferCopy copy_region = {0};
  copy_region.size = size;
  copy_region.srcOffset = offset;
  vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_region);

  vulkan_graphics_utils_end_single_time_commands(vulkan_api->device, vulkan_api->graphics_queue, vulkan_api->command_pool, command_buffer);
}

void vulkan_graphics_utils_setup_vertex_buffer(struct VulkanAPI* vulkan_api, struct Vector* vertices, VkBuffer* vertex_buffer, VkDeviceMemory* vertex_buffer_memory) {
  VkDeviceSize vertex_buffer_size = vertices->memory_size * vertices->size;
  VkBuffer vertex_staging_buffer = {0};
  VkDeviceMemory vertex_staging_buffer_memory = {0};
  vulkan_graphics_utils_create_buffer(vulkan_api->device, vulkan_api->physical_device, vertex_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &vertex_staging_buffer, &vertex_staging_buffer_memory);
  void* vertex_data;
  vkMapMemory(vulkan_api->device, vertex_staging_buffer_memory, 0, vertex_buffer_size, 0, &vertex_data);
  memcpy(vertex_data, vertices->items, vertex_buffer_size);
  vkUnmapMemory(vulkan_api->device, vertex_staging_buffer_memory);
  vulkan_graphics_utils_create_buffer(vulkan_api->device, vulkan_api->physical_device, vertex_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertex_buffer, vertex_buffer_memory);
  graphics_utils_copy_buffer(vulkan_api, vertex_staging_buffer, *vertex_buffer, vertex_buffer_size);
  vkDestroyBuffer(vulkan_api->device, vertex_staging_buffer, NULL);
  vkFreeMemory(vulkan_api->device, vertex_staging_buffer_memory, NULL);
}

void vulkan_graphics_utils_setup_vertex_buffer_pool(struct VulkanAPI* vulkan_api, struct Vector* vertices, size_t total_pool_elements, VkBuffer* vertex_buffer, VkDeviceMemory* vertex_buffer_memory) {
  VkDeviceSize vertex_buffer_size = vertices->memory_size * total_pool_elements;
  vulkan_graphics_utils_create_buffer(vulkan_api->device, vulkan_api->physical_device, vertex_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertex_buffer, vertex_buffer_memory);
}

void vulkan_graphics_utils_update_vertex_buffer(struct VulkanAPI* vulkan_api, struct Vector* vertices, VkBuffer* vertex_buffer) {
  VkDeviceSize vertex_buffer_size = vertices->memory_size * vertices->size;
  VkBuffer vertex_staging_buffer = {0};
  VkDeviceMemory vertex_staging_buffer_memory = {0};
  vulkan_graphics_utils_create_buffer(vulkan_api->device, vulkan_api->physical_device, vertex_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &vertex_staging_buffer, &vertex_staging_buffer_memory);
  void* vertex_data;
  vkMapMemory(vulkan_api->device, vertex_staging_buffer_memory, 0, vertex_buffer_size, 0, &vertex_data);
  memcpy(vertex_data, vertices->items, vertex_buffer_size);
  vkUnmapMemory(vulkan_api->device, vertex_staging_buffer_memory);
  graphics_utils_copy_buffer(vulkan_api, vertex_staging_buffer, *vertex_buffer, vertex_buffer_size);
  vkDestroyBuffer(vulkan_api->device, vertex_staging_buffer, NULL);
  vkFreeMemory(vulkan_api->device, vertex_staging_buffer_memory, NULL);
}

void vulkan_graphics_utils_setup_index_buffer(struct VulkanAPI* vulkan_api, struct Vector* indices, VkBuffer* index_buffer, VkDeviceMemory* index_buffer_memory) {
  VkDeviceSize index_buffer_size = indices->memory_size * indices->size;
  VkBuffer index_staging_buffer = {0};
  VkDeviceMemory index_staging_buffer_memory = {0};
  vulkan_graphics_utils_create_buffer(vulkan_api->device, vulkan_api->physical_device, index_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &index_staging_buffer, &index_staging_buffer_memory);
  void* index_data;
  vkMapMemory(vulkan_api->device, index_staging_buffer_memory, 0, index_buffer_size, 0, &index_data);
  memcpy(index_data, indices->items, index_buffer_size);
  vkUnmapMemory(vulkan_api->device, index_staging_buffer_memory);
  vulkan_graphics_utils_create_buffer(vulkan_api->device, vulkan_api->physical_device, index_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, index_buffer, index_buffer_memory);
  graphics_utils_copy_buffer(vulkan_api, index_staging_buffer, *index_buffer, index_buffer_size);
  vkDestroyBuffer(vulkan_api->device, index_staging_buffer, NULL);
  vkFreeMemory(vulkan_api->device, index_staging_buffer_memory, NULL);
}

void vulkan_graphics_utils_setup_index_buffer_pool(struct VulkanAPI* vulkan_api, struct Vector* indices, size_t total_pool_elements, VkBuffer* index_buffer, VkDeviceMemory* index_buffer_memory) {
  VkDeviceSize index_buffer_size = indices->memory_size * total_pool_elements;
  vulkan_graphics_utils_create_buffer(vulkan_api->device, vulkan_api->physical_device, index_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, index_buffer, index_buffer_memory);
}

void vulkan_graphics_utils_update_index_buffer(struct VulkanAPI* vulkan_api, struct Vector* indices, VkBuffer* index_buffer) {
  VkDeviceSize index_buffer_size = indices->memory_size * indices->size;
  VkBuffer index_staging_buffer;
  memset(&index_staging_buffer, 0, sizeof(VkBuffer));
  VkDeviceMemory index_staging_buffer_memory;
  memset(&index_staging_buffer_memory, 0, sizeof(VkDeviceMemory));
  vulkan_graphics_utils_create_buffer(vulkan_api->device, vulkan_api->physical_device, index_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &index_staging_buffer, &index_staging_buffer_memory);
  void* index_data;
  vkMapMemory(vulkan_api->device, index_staging_buffer_memory, 0, index_buffer_size, 0, &index_data);
  memcpy(index_data, indices->items, index_buffer_size);
  vkUnmapMemory(vulkan_api->device, index_staging_buffer_memory);
  graphics_utils_copy_buffer(vulkan_api, index_staging_buffer, *index_buffer, index_buffer_size);
  vkDestroyBuffer(vulkan_api->device, index_staging_buffer, NULL);
  vkFreeMemory(vulkan_api->device, index_staging_buffer_memory, NULL);
}

void vulkan_graphics_utils_setup_uniform_buffer(struct VulkanAPI* vulkan_api, size_t memory_size, VkBuffer* uniform_buffer, VkDeviceMemory* uniform_buffer_memory) {
  VkDeviceSize uniform_buffer_size = memory_size;
  vulkan_graphics_utils_create_buffer(vulkan_api->device, vulkan_api->physical_device, uniform_buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniform_buffer, uniform_buffer_memory);
}

uint_fast8_t vulkan_graphics_utils_setup_descriptor(struct VulkanAPI* vulkan_api, struct VkDescriptorSetLayout_T* descriptor_set_layout, struct VkDescriptorPool_T* descriptor_pool, VkDescriptorSet* descriptor_set) {
  VkDescriptorSetLayout layout;
  memset(&layout, 0, sizeof(VkDescriptorSetLayout));
  layout = descriptor_set_layout;

  VkDescriptorSetAllocateInfo alloc_info;
  memset(&alloc_info, 0, sizeof(VkDescriptorSetAllocateInfo));
  alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  alloc_info.descriptorPool = descriptor_pool;
  alloc_info.descriptorSetCount = 1;
  alloc_info.pSetLayouts = &layout;

  if (vkAllocateDescriptorSets(vulkan_api->device, &alloc_info, descriptor_set) != VK_SUCCESS) {
    log_message(LOG_SEVERITY_ERROR, "Failed to allocate descriptor sets!\n");
    return 1;
  }

  return VULKAN_API_SUCCESS;
}

VkDescriptorBufferInfo vulkan_graphics_utils_setup_descriptor_buffer_info(size_t memory_size, VkBuffer* uniform_buffer) {
  VkDescriptorBufferInfo buffer_info = {0};
  buffer_info.buffer = *uniform_buffer;
  buffer_info.offset = 0;
  buffer_info.range = memory_size;

  return buffer_info;
}

void vulkan_graphics_utils_setup_descriptor_buffer(VkWriteDescriptorSet* dcs, size_t index, VkDescriptorSet* descriptor_set, VkDescriptorBufferInfo* buffer_info) {
  dcs[index].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  dcs[index].dstSet = *descriptor_set;
  dcs[index].dstBinding = (uint32_t)index;
  dcs[index].dstArrayElement = 0;
  dcs[index].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  dcs[index].descriptorCount = 1;
  dcs[index].pBufferInfo = buffer_info;
}

VkDescriptorImageInfo vulkan_graphics_utils_setup_descriptor_image_info(VkImageView* texture_image_view, VkSampler* texture_sampler) {
  VkDescriptorImageInfo image_info = {0};
  image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  image_info.imageView = *texture_image_view;
  image_info.sampler = *texture_sampler;

  return image_info;
}

void vulkan_graphics_utils_setup_descriptor_image(VkWriteDescriptorSet* dcs, size_t index, VkDescriptorSet* descriptor_set, VkDescriptorImageInfo* image_info) {
  dcs[index].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  dcs[index].dstSet = *descriptor_set;
  dcs[index].dstBinding = (uint32_t)index;
  dcs[index].dstArrayElement = 0;
  dcs[index].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  dcs[index].descriptorCount = 1;
  dcs[index].pImageInfo = image_info;
}

VkSampleCountFlagBits vulkan_graphics_utils_get_max_msaa_samples(struct VulkanAPI* vulkan_api) {
  // return VK_SAMPLE_COUNT_1_BIT;
  VkPhysicalDeviceProperties physical_device_properties = {0};
  vkGetPhysicalDeviceProperties(vulkan_api->physical_device, &physical_device_properties);

  VkSampleCountFlags counts = physical_device_properties.limits.framebufferColorSampleCounts & physical_device_properties.limits.framebufferDepthSampleCounts;
  if (counts & VK_SAMPLE_COUNT_64_BIT)
    return VK_SAMPLE_COUNT_64_BIT;
  if (counts & VK_SAMPLE_COUNT_32_BIT)
    return VK_SAMPLE_COUNT_32_BIT;
  if (counts & VK_SAMPLE_COUNT_16_BIT)
    return VK_SAMPLE_COUNT_16_BIT;
  if (counts & VK_SAMPLE_COUNT_8_BIT)
    return VK_SAMPLE_COUNT_8_BIT;
  if (counts & VK_SAMPLE_COUNT_4_BIT)
    return VK_SAMPLE_COUNT_4_BIT;
  if (counts & VK_SAMPLE_COUNT_2_BIT)
    return VK_SAMPLE_COUNT_2_BIT;

  return VK_SAMPLE_COUNT_1_BIT;
}

VkVertexInputBindingDescription vulkan_graphics_utils_get_binding_description(uint32_t memory_size) {
  VkVertexInputBindingDescription binding_description;
  memset(&binding_description, 0, sizeof(VkVertexInputBindingDescription));
  binding_description.binding = 0;
  binding_description.stride = memory_size;
  binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  return binding_description;
}

uint_fast8_t vulkan_graphics_utils_create_descriptors(struct VulkanAPI* vulkan_api, VkDescriptorSet* descriptor_set, VkDescriptorSetLayout* descriptor_set_layout, VkDescriptorPool* descriptor_pool, uint_fast32_t descriptors) {
  VkDescriptorSetAllocateInfo alloc_info;
  memset(&alloc_info, 0, sizeof(VkDescriptorSetAllocateInfo));
  alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  alloc_info.descriptorPool = *descriptor_pool;
  alloc_info.descriptorSetCount = descriptors;
  // alloc_info.pSetLayouts = &(shader_common->shader_vulkan.descriptor_set_layout);

  // Create an array of VkDescriptorSetLayout based on the descriptorSetCount
  VkDescriptorSetLayout* layouts = (VkDescriptorSetLayout*)calloc(descriptors, sizeof(VkDescriptorSetLayout));
  for (uint_fast32_t descriptor_num = 0; descriptor_num < alloc_info.descriptorSetCount; descriptor_num++)
    layouts[descriptor_num] = *descriptor_set_layout;
  alloc_info.pSetLayouts = layouts;  // Point to the array of layouts

  if (vkAllocateDescriptorSets(vulkan_api->device, &alloc_info, descriptor_set) != VK_SUCCESS) {
    log_message(LOG_SEVERITY_ERROR, "Failed to allocate descriptor sets!\n");
    free(layouts);
    return 1;
  }

  free(layouts);

  return VULKAN_API_SUCCESS;
}
