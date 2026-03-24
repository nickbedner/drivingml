#include "drivingml/core/player.h"

void player_init(struct Player* player, uint_fast8_t player_num, uint_fast32_t window_height) {
  player->player_num = player_num;
  player_controller_init(&player->player_controller);

  player->look_at_pos = (vec3d){.x = 0, .y = 0, .z = 0};

  camera_init(&(player->camera), 1000);
  // player->camera.camera_state = CAMERA_FLY;
  player->camera.camera_state = CAMERA_LOOK_AT;

  // camera_zoom_to_target(&game->camera, planet_maximum_radius(&game->planet));
  camera_update_parameters_from_camera(&(player->camera));
  camera_look_at_zoom(&(player->camera), 2000.0, window_height);
  camera_update_camera_from_parameters(&(player->camera));
}

void player_delete(struct Player* player) {
  player_controller_delete(&player->player_controller);
}

void player_update(struct Player* player, struct ControllerAction* controller_action_list, uint_fast8_t controller_action_list_size) {
  player_controller_process_input(&player->player_controller, controller_action_list, controller_action_list_size);

  // player->camera.fly_pos = player->player_controller.pos;
  player->camera.look_at_center_point = player->look_at_pos;
  player->camera.look_at_zoom_factor = 30.0;
  player->camera.look_at_elevation = -M_PI / 16.0;
  // player->camera.look_at_azimuth += 0.01f;

  // camera_rotate(&(player->camera), -1.1f, 0.0f, 1920, 1080);
  // camera_fly_move_forward(&(player->camera), 0.1f);

  // camera_update_parameters_from_camera(&(player->camera));

  // Do camera stuff with inputs

  camera_update_camera_from_parameters(&(player->camera));
  camera_update(&(player->camera));
}
