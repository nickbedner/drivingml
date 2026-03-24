#pragma once

#include "mana/core/input/controllers/controllercommon.h"

extern const GUID GUID_DEVINTERFACE_USB_DEVICE;

uint_fast8_t gamecube_controller_init(struct ControllerCommon* controller_common);
void gamecube_controller_delete(struct ControllerCommon* controller_common);
void gamecube_controller_process_input(struct ControllerCommon* controller_common);
