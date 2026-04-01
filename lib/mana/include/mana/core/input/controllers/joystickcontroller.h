#include "mana/core/input/controllers/controllercommon.h"

u8 joystick_controller_init(struct ControllerCommon* controller_common);
void joystick_controller_delete(struct ControllerCommon* controller_common);
void joystick_controller_process_input(struct ControllerCommon* controller_common);
