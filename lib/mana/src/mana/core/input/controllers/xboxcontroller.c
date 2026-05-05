#include "mana/core/input/controllers/xboxcontroller.h"

#ifdef XBOX_CONTROLLER_SUPPORTED

#define XBOX_CONTROLLER_COUNT 4

static float xbox_controller_apply_deadzone(float value, float eps) {
  if (value > -eps && value < eps) {
    return 0.0f;
  }

  return value;
}

static float xbox_controller_normalize_stick(SHORT value) {
  if (value < 0) {
    return (float)value / 32768.0f;
  }

  return (float)value / 32767.0f;
}

static float xbox_controller_normalize_trigger(BYTE value) {
  return (float)value / 255.0f;
}

static void xbox_controller_clear_buttons(struct XboxController* xbox_controller) {
  for (i16 i = 0; i < XBOX_CONTROLLER_ACTION_LAST; i++) {
    xbox_controller->buttons[i].state = RELEASED;
    xbox_controller->buttons[i].pushed = FALSE;
    xbox_controller->buttons[i].held = FALSE;
  }
}

static void xbox_controller_process_button(struct XboxController* xbox_controller, i16 button_index, WORD button_mask) {
  WORD current_buttons = xbox_controller->current_state.Gamepad.wButtons;
  WORD previous_buttons = xbox_controller->previous_state.Gamepad.wButtons;

  b8 is_down = (current_buttons & button_mask) != 0;
  b8 was_down = (previous_buttons & button_mask) != 0;

  xbox_controller->buttons[button_index].state = is_down ? PRESSED : RELEASED;
  xbox_controller->buttons[button_index].pushed = is_down && !was_down;
  xbox_controller->buttons[button_index].held = is_down;
}

u8 xbox_controller_init(struct ControllerCommon* controller_common) {
  struct XboxController* xbox_controller = &(controller_common->xbox_controller);

  ZeroMemory(&(xbox_controller->current_state), sizeof(XINPUT_STATE));
  ZeroMemory(&(xbox_controller->previous_state), sizeof(XINPUT_STATE));

  xbox_controller->left_x = 0.0f;
  xbox_controller->left_y = 0.0f;
  xbox_controller->right_x = 0.0f;
  xbox_controller->right_y = 0.0f;
  xbox_controller->left_trigger = 0.0f;
  xbox_controller->right_trigger = 0.0f;
  xbox_controller->connected = FALSE;

  xbox_controller_clear_buttons(xbox_controller);

  if (controller_common->player_num >= XBOX_CONTROLLER_COUNT) {
    controller_common->player_num = 0;
  }

  xbox_controller->assigned_player = controller_common->player_num;

  if (XInputGetState(xbox_controller->assigned_player, &(xbox_controller->current_state)) == ERROR_SUCCESS) {
    xbox_controller->previous_state = xbox_controller->current_state;
    xbox_controller->connected = TRUE;
    return TRUE;
  }

  return FALSE;
}

void xbox_controller_delete(struct ControllerCommon* controller_common) {
  struct XboxController* xbox_controller = &(controller_common->xbox_controller);

  ZeroMemory(&(xbox_controller->current_state), sizeof(XINPUT_STATE));
  ZeroMemory(&(xbox_controller->previous_state), sizeof(XINPUT_STATE));

  xbox_controller->left_x = 0.0f;
  xbox_controller->left_y = 0.0f;
  xbox_controller->right_x = 0.0f;
  xbox_controller->right_y = 0.0f;
  xbox_controller->left_trigger = 0.0f;
  xbox_controller->right_trigger = 0.0f;
  xbox_controller->connected = FALSE;

  xbox_controller_clear_buttons(xbox_controller);
}

void xbox_controller_process_input(struct ControllerCommon* controller_common) {
  struct XboxController* xbox_controller = &(controller_common->xbox_controller);

  xbox_controller->previous_state = xbox_controller->current_state;

  if (XInputGetState(xbox_controller->assigned_player, &(xbox_controller->current_state)) != ERROR_SUCCESS) {
    xbox_controller->connected = FALSE;
    ZeroMemory(&(xbox_controller->current_state), sizeof(XINPUT_STATE));

    xbox_controller->left_x = 0.0f;
    xbox_controller->left_y = 0.0f;
    xbox_controller->right_x = 0.0f;
    xbox_controller->right_y = 0.0f;
    xbox_controller->left_trigger = 0.0f;
    xbox_controller->right_trigger = 0.0f;

    xbox_controller_clear_buttons(xbox_controller);
    return;
  }

  xbox_controller->connected = TRUE;

  xbox_controller_process_button(xbox_controller, XBOX_CONTROLLER_ACTION_DPAD_UP, XINPUT_GAMEPAD_DPAD_UP);
  xbox_controller_process_button(xbox_controller, XBOX_CONTROLLER_ACTION_DPAD_DOWN, XINPUT_GAMEPAD_DPAD_DOWN);
  xbox_controller_process_button(xbox_controller, XBOX_CONTROLLER_ACTION_DPAD_LEFT, XINPUT_GAMEPAD_DPAD_LEFT);
  xbox_controller_process_button(xbox_controller, XBOX_CONTROLLER_ACTION_DPAD_RIGHT, XINPUT_GAMEPAD_DPAD_RIGHT);
  xbox_controller_process_button(xbox_controller, XBOX_CONTROLLER_ACTION_START, XINPUT_GAMEPAD_START);
  xbox_controller_process_button(xbox_controller, XBOX_CONTROLLER_ACTION_BACK, XINPUT_GAMEPAD_BACK);
  xbox_controller_process_button(xbox_controller, XBOX_CONTROLLER_ACTION_LEFT_THUMB, XINPUT_GAMEPAD_LEFT_THUMB);
  xbox_controller_process_button(xbox_controller, XBOX_CONTROLLER_ACTION_RIGHT_THUMB, XINPUT_GAMEPAD_RIGHT_THUMB);
  xbox_controller_process_button(xbox_controller, XBOX_CONTROLLER_ACTION_LEFT_SHOULDER, XINPUT_GAMEPAD_LEFT_SHOULDER);
  xbox_controller_process_button(xbox_controller, XBOX_CONTROLLER_ACTION_RIGHT_SHOULDER, XINPUT_GAMEPAD_RIGHT_SHOULDER);
  xbox_controller_process_button(xbox_controller, XBOX_CONTROLLER_ACTION_A, XINPUT_GAMEPAD_A);
  xbox_controller_process_button(xbox_controller, XBOX_CONTROLLER_ACTION_B, XINPUT_GAMEPAD_B);
  xbox_controller_process_button(xbox_controller, XBOX_CONTROLLER_ACTION_X, XINPUT_GAMEPAD_X);
  xbox_controller_process_button(xbox_controller, XBOX_CONTROLLER_ACTION_Y, XINPUT_GAMEPAD_Y);

  xbox_controller->left_x = xbox_controller_apply_deadzone(xbox_controller_normalize_stick(xbox_controller->current_state.Gamepad.sThumbLX), XBOX_CONTROLLER_STICK_EPS);
  xbox_controller->left_y = xbox_controller_apply_deadzone(xbox_controller_normalize_stick(xbox_controller->current_state.Gamepad.sThumbLY), XBOX_CONTROLLER_STICK_EPS);
  xbox_controller->right_x = xbox_controller_apply_deadzone(xbox_controller_normalize_stick(xbox_controller->current_state.Gamepad.sThumbRX), XBOX_CONTROLLER_STICK_EPS);
  xbox_controller->right_y = xbox_controller_apply_deadzone(xbox_controller_normalize_stick(xbox_controller->current_state.Gamepad.sThumbRY), XBOX_CONTROLLER_STICK_EPS);
  xbox_controller->left_trigger = xbox_controller_apply_deadzone(xbox_controller_normalize_trigger(xbox_controller->current_state.Gamepad.bLeftTrigger), XBOX_CONTROLLER_TRIGGER_EPS);
  xbox_controller->right_trigger = xbox_controller_apply_deadzone(xbox_controller_normalize_trigger(xbox_controller->current_state.Gamepad.bRightTrigger), XBOX_CONTROLLER_TRIGGER_EPS);
}

#endif
