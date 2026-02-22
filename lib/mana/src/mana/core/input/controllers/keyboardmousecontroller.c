#include "mana/core/input/controllers/keyboardmousecontroller.h"

uint_fast8_t keyboard_mouse_controller_init(struct ControllerCommon *controller_common) {
  for (int16_t loop_num = 0; loop_num < KEY_LIMIT; loop_num++) {
    controller_common->keyboard_mouse_controller.keys[loop_num].state = RELEASED;
    controller_common->keyboard_mouse_controller.keys[loop_num].pushed = false;
    controller_common->keyboard_mouse_controller.keys[loop_num].held = false;
  }

  controller_common->keyboard_mouse_controller.mouse_x_pos = 0.0;
  controller_common->keyboard_mouse_controller.mouse_y_pos = 0.0;
  controller_common->keyboard_mouse_controller.mouse_x_pos_prev = 0.0;
  controller_common->keyboard_mouse_controller.mouse_y_pos_prev = 0.0;
  controller_common->keyboard_mouse_controller.mouse_x_pos_diff = 0.0;
  controller_common->keyboard_mouse_controller.mouse_y_pos_diff = 0.0;
  controller_common->keyboard_mouse_controller.mouse_x_pos_locked = 0.0;
  controller_common->keyboard_mouse_controller.mouse_y_pos_locked = 0.0;
  controller_common->keyboard_mouse_controller.show_cursor = true;
  controller_common->keyboard_mouse_controller.mouse_wheel = 0.0;

  return 0;
}

void keyboard_mouse_controller_delete(struct ControllerCommon *controller_common) {
}

void keyboard_mouse_controller_process_input(struct ControllerCommon *controller_common) {
  for (int16_t loop_num = 0; loop_num < KEY_LIMIT; loop_num++) {
    int16_t key_state = GetAsyncKeyState(loop_num);
    if (key_state & 0x8000) {  // If the most significant bit is set, the key is down.
      controller_common->keyboard_mouse_controller.keys[loop_num].state = PRESSED;
      controller_common->keyboard_mouse_controller.keys[loop_num].pushed = (controller_common->keyboard_mouse_controller.keys[loop_num].held == false && controller_common->keyboard_mouse_controller.keys[loop_num].pushed == false);
      controller_common->keyboard_mouse_controller.keys[loop_num].held = true;
    } else {
      controller_common->keyboard_mouse_controller.keys[loop_num].state = RELEASED;
      controller_common->keyboard_mouse_controller.keys[loop_num].pushed = false;
      controller_common->keyboard_mouse_controller.keys[loop_num].held = false;
    }
  }
}
