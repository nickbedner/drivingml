#pragma once

#include "mana/core/corecommon.h"

#ifdef _WIN64
#define KEYBOARD_MOUSE_CONTROLLER_SUPPORTED
#include <Windows.h>
#define KEY_LIMIT 512

#define KEY_A 'A'
#define KEY_B 'B'
#define KEY_C 'C'
#define KEY_D 'D'
#define KEY_E 'E'
#define KEY_F 'F'
#define KEY_G 'G'
#define KEY_H 'H'
#define KEY_I 'I'
#define KEY_J 'J'
#define KEY_K 'K'
#define KEY_L 'L'
#define KEY_M 'M'
#define KEY_N 'N'
#define KEY_O 'O'
#define KEY_P 'P'
#define KEY_Q 'Q'
#define KEY_R 'R'
#define KEY_S 'S'
#define KEY_T 'T'
#define KEY_U 'U'
#define KEY_V 'V'
#define KEY_W 'W'
#define KEY_X 'X'
#define KEY_Y 'Y'
#define KEY_Z 'Z'

#define KEY_0 '0'
#define KEY_1 '1'
#define KEY_2 '2'
#define KEY_3 '3'
#define KEY_4 '4'
#define KEY_5 '5'
#define KEY_6 '6'
#define KEY_7 '7'
#define KEY_8 '8'
#define KEY_9 '9'

#define KEY_F1 VK_F1
#define KEY_F2 VK_F2
#define KEY_F3 VK_F3
#define KEY_F4 VK_F4
#define KEY_F5 VK_F5
#define KEY_F6 VK_F6
#define KEY_F7 VK_F7
#define KEY_F8 VK_F8
#define KEY_F9 VK_F9
#define KEY_F10 VK_F10
#define KEY_F11 VK_F11
#define KEY_F12 VK_F12

#define KEY_ESC VK_ESCAPE
#define KEY_TAB VK_TAB
#define KEY_SHIFT VK_SHIFT
#define KEY_CTRL VK_CONTROL
#define KEY_ALT VK_MENU
#define KEY_SPACE VK_SPACE
#define KEY_ENTER VK_RETURN
#define KEY_BACKSPACE VK_BACK

#define KEY_LEFT_SHIFT VK_LSHIFT
#define KEY_RIGHT_SHIFT VK_RSHIFT

#define KEY_LEFT VK_LEFT
#define KEY_RIGHT VK_RIGHT
#define KEY_UP VK_UP
#define KEY_DOWN VK_DOWN

#define MOUSE_LEFT VK_LBUTTON
#define MOUSE_RIGHT VK_RBUTTON
#define MOUSE_MIDDLE VK_MBUTTON

enum KEYBOARD_MOUSE_CONTROLLER_ACTIONS {
  KEYBOARD_MOUSE_CONTROLLER_ACTION_A = 0,
  KEYBOARD_MOUSE_CONTROLLER_ACTION_B,
  KEYBOARD_MOUSE_CONTROLLER_ACTION_X,
  KEYBOARD_MOUSE_CONTROLLER_ACTION_Y,
  KEYBOARD_MOUSE_CONTROLLER_ACTION_DPAD_LEFT,
  KEYBOARD_MOUSE_CONTROLLER_ACTION_DPAD_RIGHT,
  KEYBOARD_MOUSE_CONTROLLER_ACTION_DPAD_DOWN,
  KEYBOARD_MOUSE_CONTROLLER_ACTION_DPAD_UP,
  KEYBOARD_MOUSE_CONTROLLER_ACTION_START,
  KEYBOARD_MOUSE_CONTROLLER_ACTION_Z,
  KEYBOARD_MOUSE_CONTROLLER_ACTION_R,
  KEYBOARD_MOUSE_CONTROLLER_ACTION_L,
  KEYBOARD_MOUSE_CONTROLLER_ACTION_MOUSE_X,
  KEYBOARD_MOUSE_CONTROLLER_ACTION_MOUSE_Y,
  KEYBOARD_MOUSE_CONTROLLER_ACTION_L_TRIGGER,
  KEYBOARD_MOUSE_CONTROLLER_ACTION_R_TRIGGER,
  KEYBOARD_MOUSE_CONTROLLER_ACTION_LAST
};

#define XBOX_CONTROLLER_SUPPORTED
#include <Xinput.h>

#define PLAYSATION_CONTROLLER_SUPPORTED

#define GAMECUBE_CONTROLLER_SUPPORTED
#include <SetupAPI.h>
#include <cfgmgr32.h>
#include <winusb.h>

enum GC_CONTROLLER_ACTIONS {
  GC_CONTROLLER_ACTION_A = 0,
  GC_CONTROLLER_ACTION_B,
  GC_CONTROLLER_ACTION_X,
  GC_CONTROLLER_ACTION_Y,
  GC_CONTROLLER_ACTION_DPAD_LEFT,
  GC_CONTROLLER_ACTION_DPAD_RIGHT,
  GC_CONTROLLER_ACTION_DPAD_DOWN,
  GC_CONTROLLER_ACTION_DPAD_UP,
  GC_CONTROLLER_ACTION_START,
  GC_CONTROLLER_ACTION_Z,
  GC_CONTROLLER_ACTION_R,
  GC_CONTROLLER_ACTION_L,
  GC_CONTROLLER_ACTION_JOYSTICK_X,
  GC_CONTROLLER_ACTION_JOYSTICK_Y,
  GC_CONTROLLER_ACTION_CSTICK_X,
  GC_CONTROLLER_ACTION_CSTICK_Y,
  GC_CONTROLLER_ACTION_L_TRIGGER,
  GC_CONTROLLER_ACTION_R_TRIGGER,
  GC_CONTROLLER_ACTION_LAST
};

// Gamecube Controller
#define CONTROLLER_VID 0x057E
#define CONTROLLER_PID 0x0337

// Data start 1 byte + (controller plugged in byte + 8 bytes of data) x 4
#define GC_ADAPTER_TOTAL_BUFFER_SIZE 37
#define GC_BUFFER_SIZE 8

#define GC_ADAPTER_START_OF_INPUT 33
#define GC_CONTROLLER_PLUGGED_IN 20
#define GC_CONTROLLER_NOT_PLUGGED_IN 4
// Buffer 2
#define GC_CONTROLLER_A 0x01
#define GC_CONTROLLER_B 0x02
#define GC_CONTROLLER_X 0x04
#define GC_CONTROLLER_Y 0x08
#define GC_CONTROLLER_DPAD_LEFT 0x10
#define GC_CONTROLLER_DPAD_RIGHT 0x20
#define GC_CONTROLLER_DPAD_DOWN 0x40
#define GC_CONTROLLER_DPAD_UP 0x80
// Buffer 3
#define GC_CONTROLLER_START 0x01
#define GC_CONTROLLER_Z 0x02
#define GC_CONTROLLER_R 0x04
#define GC_CONTROLLER_L 0x08
// Buffer 4 Joystick X
// Buffer 5 Joystick Y
// Buffer 6 C-Stick X
// Buffer 7 C-Stick Y
// Buffer 8 L Trigger
// Buffer 9 R Trigger

#define JOYSTICK_CONTROLLER_SUPPORTED

#endif

enum CONTROLLER_TYPE {
  CONTROLLER_NONE = 0,
#ifdef KEYBOARD_MOUSE_CONTROLLER_SUPPORTED
  CONTROLLER_KEYBOARD_MOUSE,
#endif
#ifdef XBOX_CONTROLLER_SUPPORTED
  CONTROLLER_XBOX,
#endif
#ifdef PLAYSATION_CONTROLLER_SUPPORTED
  CONTROLLER_PLAYSTATION,
#endif
#ifdef GAMECUBE_CONTROLLER_SUPPORTED
  CONTROLLER_GAMECUBE,
#endif
#ifdef JOYSTICK_CONTROLLER_SUPPORTED
  CONTROLLER_JOYSTICK,
#endif
  CONTROLLER_LAST
};

#ifdef KEYBOARD_MOUSE_CONTROLLER_SUPPORTED
enum KeyState { PRESSED,
                RELEASED };

struct Key {
  enum KeyState state;
  b8 pushed;
  b8 held;
};

struct KeyboardMouseController {
  struct Key keys[KEY_LIMIT];
  r64 mouse_x_pos;
  r64 mouse_y_pos;
  r64 mouse_x_pos_prev;
  r64 mouse_y_pos_prev;
  r64 mouse_x_pos_diff;
  r64 mouse_y_pos_diff;
  r64 mouse_x_pos_locked;
  r64 mouse_y_pos_locked;
  r64 mouse_wheel;
  b8 show_cursor;
  b8 lock_cursor;
  b8 locked;
};
#endif

#ifdef XBOX_CONTROLLER_SUPPORTED
struct XboxController {
#ifdef _WIN64
  XINPUT_STATE current_state;
  XINPUT_STATE previous_state;

  u8 assigned_player;
#endif
};
#endif

#ifdef PLAYSATION_CONTROLLER_SUPPORTED
struct PlaystationController {
  r32 placeholder;
};
#endif

#ifdef GAMECUBE_CONTROLLER_SUPPORTED
struct GamecubeController {
#ifdef _WIN64
  // --- 8-byte aligned first ---
  WINUSB_INTERFACE_HANDLE winusb_handle;
  OVERLAPPED overlapped;
  BYTE buffer[GC_ADAPTER_TOTAL_BUFFER_SIZE];
  UCHAR read_pipe_id;
  UCHAR write_pipe_id;
  b8 reading_pending;
  // Note: Padding for 4 byte grouping
  u8 _pad0;

#else
  r32 placeholder;
#endif
};
#endif

#ifdef JOYSTICK_CONTROLLER_SUPPORTED
struct JoystickController {
  r32 placeholder;
};
#endif

struct ControllerAction {
  u8 button;
  b8 pressed;
  b8 held;
  b8 released;
  // Note: For pressure sensitive buttons and normalized joystick values?
  r32 value;
};

struct ControllerCommon {
  union {
#ifdef KEYBOARD_MOUSE_CONTROLLER_SUPPORTED
    struct KeyboardMouseController keyboard_mouse_controller;
#endif
#ifdef XBOX_CONTROLLER_SUPPORTED
    struct XboxController xbox_controller;
#endif
#ifdef PLAYSATION_CONTROLLER_SUPPORTED
    struct PlaystationController playstation_controller;
#endif
#ifdef GAMECUBE_CONTROLLER_SUPPORTED
    struct GamecubeController gamecube_controller;
#endif
#ifdef JOYSTICK_CONTROLLER_SUPPORTED
    struct JoystickController joystick_controller;
#endif
  };

  struct ControllerAction controller_action_list[16];
  enum CONTROLLER_TYPE controller_type;
  // Note: player 0 is unassigned
  u8 player_num;
  u8 controller_action_list_size;
};
