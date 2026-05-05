#include "mana/core/input/inputmanager.h"

// Note: Register all controllers to player 1 only if playing solo, otherwise world select allow hop in and then let them select controller they're using
void input_manager_init(struct InputManager* input_manager, struct Surface* surface) {
  array_list_init(&(input_manager->controllers));
  // TODO: For now hard code keyboard and mouse till I know how to find one
#ifdef _WIN64
  struct Controller* keyboard_mouse_controller = (struct Controller*)calloc(1, sizeof(struct Controller));
  keyboard_mouse_controller->controller_common.controller_type = CONTROLLER_KEYBOARD_MOUSE;
  controller_init(keyboard_mouse_controller);  // Keyboard and mouse controller
  array_list_add(&(input_manager->controllers), keyboard_mouse_controller);

  struct Controller* xbox_controller = (struct Controller*)calloc(1, sizeof(struct Controller));
  xbox_controller->controller_common.controller_type = CONTROLLER_XBOX;
  controller_init(xbox_controller);  // Xbox controller
  array_list_add(&(input_manager->controllers), xbox_controller);
#endif
#ifdef GAMECUBE_CONTROLLER_SUPPORTED
  // TODO: This is good code and I need to finish this but at a later time
  // struct Controller *gamecube_controller = calloc(1, sizeof(struct Controller));
  // gamecube_controller->controller_common.controller_type = CONTROLLER_GAMECUBE;
  // controller_init(gamecube_controller);  // Gamecube controller
  // array_list_add(&(input_manager->controllers), gamecube_controller);
  // gamecube_controller->controller_common.player_num = 1;
#endif
  // Enumerate all controllers, xbox, playstation, and gamecube controllers get rumble support, otherwise controller gets relegated to generic joystick
}

void input_manager_delete(struct InputManager* input_manager) {
  for (size_t i = 0; i < input_manager->controllers.size; i++) {
    struct Controller* controller = (struct Controller*)array_list_get(&(input_manager->controllers), i);
    controller_delete(controller);
    free(controller);
  }
  array_list_delete(&(input_manager->controllers));
}

void input_manager_process_input(struct InputManager* input_manager) {
  for (size_t i = 0; i < input_manager->controllers.size; i++) {
    struct Controller* controller = (struct Controller*)array_list_get(&(input_manager->controllers), i);
    controller_process_input(controller);
  }
}

#ifdef KEYBOARD_MOUSE_CONTROLLER_SUPPORTED
void input_manager_show_cursor(struct InputManager* input_manager, b8 show_cursor) {
  struct KeyboardMouseController* input_manager_keyboard_mouse = input_manager_find_keyboard_mouse(input_manager);
  input_manager_keyboard_mouse->show_cursor = show_cursor;
#ifdef _WIN64
  // if (show_cursor == FALSE)
  //   SetCursor(NULL);
  //  if (show_cursor)
  //    SetCursor(LoadCursor(NULL, IDC_ARROW)); // Restore the default arrow cursor
  //  else
  //    SetCursor(NULL); // Hide the cursor
#elif defined(__linux__)
#elif defined(__APPLE__)
#endif
}

void input_manager_lock_cursor(struct InputManager* input_manager, struct Surface* surface, b8 lock_cursor) {
  struct KeyboardMouseController* input_manager_keyboard_mouse = input_manager_find_keyboard_mouse(input_manager);
#ifdef _WIN64
  // Only lock the cursor if the window is in focus
  if (GetForegroundWindow() == surface->hwnd) {
    input_manager_keyboard_mouse->lock_cursor = lock_cursor;
    if (lock_cursor) {
      if (input_manager_keyboard_mouse->locked == FALSE) {
        input_manager_keyboard_mouse->mouse_x_pos_diff = 0;
        input_manager_keyboard_mouse->mouse_y_pos_diff = 0;

        POINT locked_position = {0, 0};
        GetCursorPos(&locked_position);

        input_manager_move_cursor_to_center(input_manager, surface);

        input_manager_keyboard_mouse->mouse_x_pos_locked = (r64)locked_position.x;
        input_manager_keyboard_mouse->mouse_y_pos_locked = (r64)locked_position.y;

        SetCapture(GetActiveWindow());  // Capture all subsequent mouse input to this window
        input_manager_keyboard_mouse->locked = TRUE;
      }
    } else {
      if (input_manager_keyboard_mouse->locked == TRUE) {
        ReleaseCapture();  // Release the mouse capture
        input_manager_keyboard_mouse->locked = FALSE;
        SetCursorPos((i32)(input_manager_keyboard_mouse->mouse_x_pos_locked), (i32)(input_manager_keyboard_mouse->mouse_y_pos_locked));
      }
    }
  }
#elif defined(__linux__)
#elif defined(__APPLE__)
#endif
}

b8 input_manager_in_window(struct InputManager* input_manager, struct Surface* surface) {
#ifdef _WIN64
  if (GetForegroundWindow() == surface->hwnd) {
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(surface->hwnd, &pt);
    RECT rect;
    GetClientRect(surface->hwnd, &rect);
    if (PtInRect(&rect, pt))
      return TRUE;
    else
      return FALSE;
  } else
    return FALSE;
#elif defined(__linux__)
#elif defined(__APPLE__)
#endif
}

vec2 input_manager_move_cursor_to_center(struct InputManager* input_manager, struct Surface* surface) {
#ifdef _WIN64
  RECT client_rect;
  GetClientRect(surface->hwnd, &client_rect);
  i32 center_x = (client_rect.left + client_rect.right) / 2;
  i32 center_y = (client_rect.top + client_rect.bottom) / 2;
  POINT center_point;
  center_point.x = center_x;
  center_point.y = center_y;
  ClientToScreen(surface->hwnd, &center_point);
  SetCursorPos(center_point.x, center_point.y);
  return (vec2){.x = (r32)center_point.x, .y = (r32)center_point.y};
#elif defined(__linux__)
#elif defined(__APPLE__)
#endif
}

b8 input_manager_is_window_in_focus(struct Surface* surface) {
#ifdef _WIN64
  return (GetForegroundWindow() == surface->hwnd);
#elif defined(__linux__)
  return FALSE;
  // Linux-specific implementation
#elif defined(__APPLE__)
  return FALSE;
  // macOS-specific implementation
#endif
}

// Temp because I need to go to bed
struct ControllerAction* input_manager_get_controller_actions(struct InputManager* input_manager) {
  for (size_t i = 0; i < input_manager->controllers.size; i++) {
    struct Controller* controller = (struct Controller*)array_list_get(&(input_manager->controllers), i);
    if (controller->controller_common.controller_type == CONTROLLER_GAMECUBE)
      return controller->controller_common.controller_action_list;
  }
  return NULL;
}
u8 input_manager_get_controller_action_list_length(struct InputManager* input_manager) {
  for (size_t i = 0; i < input_manager->controllers.size; i++) {
    struct Controller* controller = (struct Controller*)array_list_get(&(input_manager->controllers), i);
    if (controller->controller_common.controller_type == CONTROLLER_GAMECUBE)
      return controller->controller_common.controller_action_list_size;
  }
  return 0;
}

struct KeyboardMouseController* input_manager_find_keyboard_mouse(struct InputManager* input_manager) {
  for (size_t i = 0; i < input_manager->controllers.size; i++) {
    struct Controller* controller = (struct Controller*)array_list_get(&(input_manager->controllers), i);
    if (controller->controller_common.controller_type == CONTROLLER_KEYBOARD_MOUSE)
      return &(controller->controller_common.keyboard_mouse_controller);
  }
  return NULL;
}

// void input_manager_set_cursor_pos(struct InputManager *input_manager, r64 x_pos, r64 y_pos) {
//   struct Controller *keyboard_mouse_controller = input_manager_find_keyboard_mouse(input_manager);
//   if (keyboard_mouse_controller == NULL)
//     return;
//
//   keyboard_mouse_controller->controller_common.keyboard_mouse_controller.mouse_x_pos_prev = keyboard_mouse_controller->controller_common.keyboard_mouse_controller.mouse_x_pos;
//   keyboard_mouse_controller->controller_common.keyboard_mouse_controller.mouse_y_pos_prev = keyboard_mouse_controller->controller_common.keyboard_mouse_controller.mouse_y_pos;
//
//   keyboard_mouse_controller->controller_common.keyboard_mouse_controller.mouse_x_pos = x_pos;
//   keyboard_mouse_controller->controller_common.keyboard_mouse_controller.mouse_y_pos = y_pos;
// }
//
// void input_manager_set_cursor_scroll(struct InputManager *input_manager, r64 wheel_scroll) {
//   struct Controller *keyboard_mouse_controller = input_manager_find_keyboard_mouse(input_manager);
//   if (keyboard_mouse_controller == NULL)
//     return;
//
//   keyboard_mouse_controller->controller_common.keyboard_mouse_controller.mouse_wheel = wheel_scroll;
// }
#endif
