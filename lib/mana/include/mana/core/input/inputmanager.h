#pragma once

#include "mana/core/input/controllers/controller.h"
#include "mana/graphics/render/surface.h"

struct InputManager {
  struct ArrayList controllers;
  // If the keyboard and mouse is supported we need to handle callbacks for them
#ifdef KEYBOARD_MOUSE_CONTROLLER_SUPPORTED
#endif
};

void input_manager_init(struct InputManager *input_manager, struct Surface *surface);
void input_manager_delete(struct InputManager *input_manager);
void input_manager_process_input(struct InputManager *input_manager);

#ifdef KEYBOARD_MOUSE_CONTROLLER_SUPPORTED
void input_manager_show_cursor(struct InputManager *input_manager, bool show_cursor);
void input_manager_lock_cursor(struct InputManager *input_manager, struct Surface *surface, bool lock_cursor);
bool input_manager_in_window(struct InputManager *input_manager, struct Surface *surface);

vec2 input_manager_move_cursor_to_center(struct InputManager *input_manager, struct Surface *surface);
bool input_manager_is_window_in_focus(struct Surface *surface);

struct ControllerAction *input_manager_get_controller_actions(struct InputManager *input_manager);
uint_fast8_t input_manager_get_controller_action_list_length(struct InputManager *input_manager);
struct KeyboardMouseController *input_manager_find_keyboard_mouse(struct InputManager *input_manager);
// void input_manager_set_cursor_pos(struct InputManager *input_manager, double x_pos, double y_pos);
// void input_manager_set_cursor_scroll(struct InputManager *input_manager, double wheel_scroll);
#endif
