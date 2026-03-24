#pragma once

#include "mana/core/input/inputmanager.h"
#include "mana/graphics/render/renderer/renderer.h"

struct APICommon;

struct Window {
  struct APICommon* api_common;
  struct SwapChain* swap_chain;
  struct GBuffer* gbuffer;
  struct PostProcess* post_process;
  char* title;

  struct Renderer renderer;
  struct Surface surface;
  struct InputManager input_manager;

  float delta_time;

  bool framebuffer_resized;
  bool should_resize;
  bool new_window;
  bool minimized;
  bool should_close;
  bool vsync;
};

enum {
  WINDOW_SUCCESS = 0,
  WINDOW_ERROR,
  WINDOW_LAST_ERROR
};

uint_fast8_t window_init(struct Window* window, struct APICommon* api_common, char* title, struct RendererSettings* renderer_settings);
void window_delete(struct Window* window);
void window_prepare_frame(struct Window* window);
void window_end_frame(struct Window* window);
void window_recreate_swap_chain(struct Window* window);
