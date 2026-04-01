#pragma once

#include "mana/core/input/controllers/controllercommon.h"

u8 xbox_controller_init(struct ControllerCommon* controller_common);
void xbox_controller_delete(struct ControllerCommon* controller_common);
void xbox_controller_process_input(struct ControllerCommon* controller_common);
