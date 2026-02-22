#pragma once

#include "drivingml/core/playercontroller.h"
#include "mana/graphics/utilities/camera.h"


struct Player {
  uint_fast8_t player_num;
  // For this Unity went with OO over ECS
  struct PlayerController player_controller;
  // This should all be ECS stuff I'm guessing, attach a camera to the player entity
  vec3d look_at_pos;
  struct Camera camera;
};

void player_init(struct Player* player, uint_fast8_t player_num, uint_fast32_t window_height);
void player_delete(struct Player* player);
void player_update(struct Player* player, struct ControllerAction* controller_action_list, uint_fast8_t controller_action_list_size);
