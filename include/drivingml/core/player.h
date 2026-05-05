#pragma once

#include "drivingml/core/playercontroller.h"
#include "mana/core/graphics/utilities/camera.h"

struct Player {
  // For this Unity went with OO over ECS
  struct PlayerController player_controller;
  struct Camera camera;
  // This should all be ECS stuff I'm guessing, attach a camera to the player entity
  vec3d look_at_pos;
  r64 look_at_azimuth;
  r64 look_at_elevation;
  u8 player_num;
};

void player_init(struct Player* player, u8 player_num, uint_fast32_t window_height);
void player_delete(struct Player* player);
void player_update(struct Player* player, struct ControllerAction* controller_action_list, u8 controller_action_list_size);
