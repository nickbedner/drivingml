#pragma once

#include "mana/core/input/controllers/controllercommon.h"

uint_fast8_t xbox_controller_init(struct ControllerCommon *controller_common);
void xbox_controller_delete(struct ControllerCommon *controller_common);
void xbox_controller_process_input(struct ControllerCommon *controller_common);
