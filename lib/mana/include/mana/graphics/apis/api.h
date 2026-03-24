#pragma once

#include "mana/graphics/apis/apicommon.h"

#ifdef VULKAN_API_SUPPORTED
#include "mana/graphics/apis/apivulkan.h"
#endif
#ifdef DIRECTX_12_API_SUPPORTED
#include "mana/graphics/apis/apidirectx12.h"
#endif

// TODO: This should be full of generic functions that are used by the graphics library

struct APIFunc {
  uint_fast8_t (*api_init)(struct APICommon* api_common);
  void (*api_delete)(struct APICommon* api_common);
};

static const struct APIFunc API_NONE_FUNC = {.api_init = NULL, .api_delete = NULL};
#ifdef VULKAN_API_SUPPORTED
static const struct APIFunc API_VULKAN_FUNC = {.api_init = api_vulkan_init, .api_delete = api_vulkan_delete};
#endif
#ifdef DIRECTX_12_API_SUPPORTED
static const struct APIFunc API_DIRECTX_12_FUNC = {.api_init = api_directx_12_init, .api_delete = api_directx_12_delete};
#endif

struct API {
  struct APIFunc api_func;
  struct APICommon api_common;
};

struct APIHandler {
  struct APIFunc api_func;
  enum API_TYPE api_type;
  bool inverted_y;
};

// Array of API handlers
static const struct APIHandler api_handlers[API_LAST + 1] = {
    {API_NONE_FUNC, API_NONE, false},
#ifdef VULKAN_API_SUPPORTED
    {API_VULKAN_FUNC, API_VULKAN, true},
#endif
#ifdef DIRECTX_12_API_SUPPORTED
    {API_DIRECTX_12_FUNC, API_DIRECTX_12, false},
#endif
#ifdef METAL_API_SUPPORTED
    {API_APPLE_METAL_FUNC, API_APPLE_METAL, true},
#endif
    {API_NONE_FUNC, API_LAST, false}  // Sentinel value to mark end of array
};

uint_fast8_t gpu_api_init(struct API* api, enum API_TYPE preferred_api_type);
void gpu_api_delete(struct API* api);
