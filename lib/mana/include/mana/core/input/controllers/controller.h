#pragma once

#include "mana/core/input/controllers/controllercommon.h"

#ifdef KEYBOARD_MOUSE_CONTROLLER_SUPPORTED
#include "mana/core/input/controllers/keyboardmousecontroller.h"
#endif
#ifdef XBOX_CONTROLLER_SUPPORTED
#include "mana/core/input/controllers/xboxcontroller.h"
#endif
#ifdef PLAYSATION_CONTROLLER_SUPPORTED
#include "mana/core/input/controllers/playstationcontroller.h"
#endif
#ifdef GAMECUBE_CONTROLLER_SUPPORTED
#include "mana/core/input/controllers/gamecubecontroller.h"
#endif
#ifdef JOYSTICK_CONTROLLER_SUPPORTED
#include "mana/core/input/controllers/joystickcontroller.h"
#endif

struct ControllerFunc {
  u8 (*controller_init)(struct ControllerCommon*);
  void (*controller_delete)(struct ControllerCommon*);
  void (*controller_process_input)(struct ControllerCommon*);
};

#ifdef KEYBOARD_MOUSE_CONTROLLER_SUPPORTED
global const struct ControllerFunc KEYBOARD_MOUSE_CONTROLLER = {keyboard_mouse_controller_init, keyboard_mouse_controller_delete, keyboard_mouse_controller_process_input};
#endif
#ifdef XBOX_CONTROLLER_SUPPORTED
global const struct ControllerFunc XBOX_CONTROLLER = {xbox_controller_init, xbox_controller_delete, xbox_controller_process_input};
#endif
#ifdef PLAYSATION_CONTROLLER_SUPPORTED
global const struct ControllerFunc PLAYSTATION_CONTROLLER = {playstation_controller_init, playstation_controller_delete, playstation_controller_process_input};
#endif
#ifdef GAMECUBE_CONTROLLER_SUPPORTED
global const struct ControllerFunc GAMECUBE_CONTROLLER = {gamecube_controller_init, gamecube_controller_delete, gamecube_controller_process_input};
#endif
#ifdef JOYSTICK_CONTROLLER_SUPPORTED
global const struct ControllerFunc JOYSTICK_CONTROLLER = {joystick_controller_init, joystick_controller_delete, joystick_controller_process_input};
#endif

struct Controller {
  struct ControllerFunc controller_func;
  struct ControllerCommon controller_common;
};

u8 controller_init(struct Controller* controller);
void controller_delete(struct Controller* controller);
void controller_process_input(struct Controller* controller);
