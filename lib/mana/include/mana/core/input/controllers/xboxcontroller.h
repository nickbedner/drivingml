#pragma once

#include "mana/core/input/controllers/controllercommon.h"

#define XBOX_CONTROLLER_STICK_EPS 0.25f
#define XBOX_CONTROLLER_TRIGGER_EPS 0.05f

u8 xbox_controller_init(struct ControllerCommon* controller_common);
void xbox_controller_delete(struct ControllerCommon* controller_common);
void xbox_controller_process_input(struct ControllerCommon* controller_common);
