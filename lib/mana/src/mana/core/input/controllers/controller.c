#include "mana/core/input/controllers/controller.h"

u8 controller_init(struct Controller* controller) {
#ifdef KEYBOARD_MOUSE_CONTROLLER_SUPPORTED
  if (controller->controller_common.controller_type == CONTROLLER_KEYBOARD_MOUSE)
    controller->controller_func = KEYBOARD_MOUSE_CONTROLLER;
#endif
#ifdef XBOX_CONTROLLER_SUPPORTED
  if (controller->controller_common.controller_type == CONTROLLER_XBOX)
    controller->controller_func = XBOX_CONTROLLER;
#endif
#ifdef PLAYSATION_CONTROLLER_SUPPORTED
  if (controller->controller_common.controller_type == CONTROLLER_PLAYSTATION)
    controller->controller_func = PLAYSTATION_CONTROLLER;
#endif
#ifdef GAMECUBE_CONTROLLER_SUPPORTED
  if (controller->controller_common.controller_type == CONTROLLER_GAMECUBE)
    controller->controller_func = GAMECUBE_CONTROLLER;
#endif
#ifdef JOYSTICK_CONTROLLER_SUPPORTED
  if (controller->controller_common.controller_type == CONTROLLER_JOYSTICK)
    controller->controller_func = JOYSTICK_CONTROLLER;
#endif

  return controller->controller_func.controller_init(&(controller->controller_common));
}

void controller_delete(struct Controller* controller) {
  controller->controller_func.controller_delete(&(controller->controller_common));
}

void controller_process_input(struct Controller* controller) {
  controller->controller_func.controller_process_input(&(controller->controller_common));
}
