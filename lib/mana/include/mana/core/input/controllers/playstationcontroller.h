#include "mana/core/input/controllers/controllercommon.h"

u8 playstation_controller_init(struct ControllerCommon* controller_common);
void playstation_controller_delete(struct ControllerCommon* controller_common);
void playstation_controller_process_input(struct ControllerCommon* controller_common);
