#include "mana/core/input/controllers/xboxcontroller.h"

uint_fast8_t xbox_controller_init(struct ControllerCommon *controller_common) {
  return 0;
}

void xbox_controller_delete(struct ControllerCommon *controller_common) {
}

void xbox_controller_process_input(struct ControllerCommon *controller_common) {
  // Get the state of the first controller, limited to 0-3
  // https://learn.microsoft.com/en-us/windows/win32/api/xinput/nf-xinput-xinputgetstate
  if (XInputGetState(controller_common->player_num, &(controller_common->xbox_controller.current_state)) == ERROR_SUCCESS) {
    // Controller is connected
    if (controller_common->xbox_controller.current_state.Gamepad.wButtons & XINPUT_GAMEPAD_A) {
      // 'A' button is pressed
      printf("A button pressed\n");
    }
  } else {
    // Controller is not connected
    // printf("Controller not connected\n");
  }

  controller_common->xbox_controller.previous_state = controller_common->xbox_controller.current_state;
}
