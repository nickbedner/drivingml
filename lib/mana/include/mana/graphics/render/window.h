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

  r32 delta_time;

  b8 framebuffer_resized;
  b8 should_resize;
  b8 new_window;
  b8 minimized;
  b8 should_close;
  b8 vsync;
};

enum {
  WINDOW_SUCCESS = 0,
  WINDOW_ERROR,
  WINDOW_LAST_ERROR
};

u8 window_init(struct Window* window, struct APICommon* api_common, char* title, struct RendererSettings* renderer_settings);
void window_delete(struct Window* window);
void window_prepare_frame(struct Window* window);
void window_end_frame(struct Window* window);
void window_recreate_swap_chain(struct Window* window);
