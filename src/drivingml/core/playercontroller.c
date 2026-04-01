#include "drivingml/core/playercontroller.h"

void player_controller_init(struct PlayerController* player_controller) {
  // Assign current player controller to gamecube default
  // memcpy(player_controller->input_to_actions_map_main, PLAYER_CONTROLLER_DEFAULT_GAMECUBE_BINDINGS, sizeof(PLAYER_CONTROLLER_DEFAULT_GAMECUBE_BINDINGS));
  for (u8 i = 0; i < PLAYER_CONTROLLER_LAST; i++)
    player_controller->input_to_actions_map_main[i] = (enum PLAYER_CONTROLLER_ACTION)PLAYER_CONTROLLER_DEFAULT_GAMECUBE_BINDINGS[i];
}

void player_controller_delete(struct PlayerController* player_controller) {
}

// Should action and value
void player_controller_process_input(struct PlayerController* player_controller, struct ControllerAction* controller_action_list, u8 controller_action_list_size) {
  for (u8 i = 0; i < controller_action_list_size; i++) {
    struct ControllerAction controller_action = controller_action_list[i];
    for (u8 j = 0; j < PLAYER_CONTROLLER_LAST; j++) {
      u8 player_binding = (u8)player_controller->input_to_actions_map_main[j];
      if (controller_action.button == player_binding) {  // || action == player_controller->input_to_actions_map_alt[j]) {
        // Do action
        switch (j) {
            // Note: These should technically apply an impulse to the player's physics object in the direction of the movement
          case PLAYER_CONTROLLER_MOVE_FORWARD:
            if (controller_action.value > 0)
              player_controller->pos.y += (r64)controller_action.value;
            break;
          case PLAYER_CONTROLLER_MOVE_BACKWARD:
            if (controller_action.value < 0)
              player_controller->pos.y += (r64)controller_action.value;
            break;
          case PLAYER_CONTROLLER_MOVE_LEFT:
            if (controller_action.value < 0)
              player_controller->pos.x += (r64)controller_action.value;
            break;
          case PLAYER_CONTROLLER_MOVE_RIGHT:
            if (controller_action.value > 0)
              player_controller->pos.x += (r64)controller_action.value;
            break;
          case PLAYER_CONTROLLER_CAMERA_UP:
            break;
          case PLAYER_CONTROLLER_CAMERA_DOWN:
            break;
          case PLAYER_CONTROLLER_CAMERA_LEFT:
            break;
          case PLAYER_CONTROLLER_CAMERA_RIGHT:
            break;
          case PLAYER_CONTROLLER_INTERACT:
            break;
          case PLAYER_CONTROLLER_JUMP:
            break;
          case PLAYER_CONTROLLER_SPRINT:
            break;
          case PLAYER_CONTROLLER_CROUCH:
            break;
          case PLAYER_CONTROLLER_LEFT_ATTACK:
            break;
          case PLAYER_CONTROLLER_RIGHT_ATTACK:
            break;
          case PLAYER_CONTROLLER_PUT_AWAY_WEAPONS:
            break;
          case PLAYER_CONTROLLER_PHYSICS_GRAB:
            break;
          case PLAYER_CONTROLLER_GAME_MENU:
            break;
          case PLAYER_CONTROLLER_PLAYER_MENU:
            break;
          case PLAYER_CONTROLLER_FAVORITES_MENU:
            break;
          case PLAYER_CONTROLLER_CAMERA_CHANGE:
            break;
          case PLAYER_CONTROLLER_AUTO_RUN:
            break;
          case PLAYER_CONTROLLER_WALK:
            break;
          case PLAYER_CONTROLLER_CHAT:
            break;
          case PLAYER_CONTROLLER_HOTKEY_1:
            break;
          case PLAYER_CONTROLLER_HOTKEY_2:
            break;
          case PLAYER_CONTROLLER_HOTKEY_3:
            break;
          case PLAYER_CONTROLLER_HOTKEY_4:
            break;
          case PLAYER_CONTROLLER_HOTKEY_5:
            break;
          case PLAYER_CONTROLLER_HOTKEY_6:
            break;
          case PLAYER_CONTROLLER_HOTKEY_7:
            break;
          case PLAYER_CONTROLLER_HOTKEY_8:
            break;
          case PLAYER_CONTROLLER_HOTKEY_9:
            break;
          case PLAYER_CONTROLLER_LAST:
            break;
          default:
            break;
        }
      }
    }
  }
}
