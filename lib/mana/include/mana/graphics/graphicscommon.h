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

#include "mana/math/vec3.h"
#include "mana/math/vec4.h"

static const vec4 COLOR_TRANSPARENT_BLACK = {.x = 0.0f, .y = 0.0f, .z = 0.0f, .w = 0.0f};
static const vec4 COLOR_CORNFLOWER_BLUE = {.x = 0.392f, .y = 0.584f, .z = 0.929f, .w = 1.0f};

struct LightingUniformBufferObject {
  vec3 direction;
  vec3 ambient_color;
  vec3 diffuse_colour;
  vec3 specular_colour;
};
