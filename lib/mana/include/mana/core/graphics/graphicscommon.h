#pragma once

#ifdef _WIN64
#define VULKAN_API_SUPPORTED
#define DIRECTX_12_API_SUPPORTED
#elif defined(__linux__)
#define VULKAN_API_SUPPORTED
#elif defined(__APPLE__)
#define METAL_API_SUPPORTED
#elif defined(__ANDROID__)
#define VULKAN_API_SUPPORTED
#endif

#define MAX_FRAMES_IN_FLIGHT 2
#define MAX_SWAP_CHAIN_FRAMES MAX_FRAMES_IN_FLIGHT + 1
#define POST_PROCESS_PING_PONG 2

// Note: Inifite z far is used but still have this for reference. Not strictly infinite, but sufficiently large to be considered practically infinite for most intents and purposes
#define Z_FAR 10000000.0f
#define Z_NEAR 0.01f

#include <stdalign.h>

#include "mana/core/math/vec3.h"
#include "mana/core/math/vec4.h"

global const vec4 COLOR_TRANSPARENT_BLACK = {.x = 0.0f, .y = 0.0f, .z = 0.0f, .w = 0.0f};
global const vec4 COLOR_SUN_BLUE = {.x = 0.0f, .y = 0.5098f, .z = 0.8627f, .w = 1.0f};
global const vec4 COLOR_CORNFLOWER_BLUE = {.x = 0.392f, .y = 0.584f, .z = 0.929f, .w = 1.0f};

struct LightingUniformBufferObject {
  alignas(16) vec4 direction;
  alignas(16) vec4 ambient_color;
  alignas(16) vec4 diffuse_color;
  alignas(16) vec4 specular_color;
};
