#define WIN32_LEAN_AND_MEAN
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <Windows.h>

////////

#include "drivingml/game.h"

static int recv_all(SOCKET sock, char* buffer, int size) {
  int total = 0;
  int bytes;

  while (total < size) {
    bytes = recv(sock, buffer + total, size - total, 0);
    if (bytes <= 0) return bytes;
    total += bytes;
  }
  return total;
}

void game_init(struct Game* game, struct Mana* mana, struct Window* window) {
  game->window = window;

  game->previous_reward = 0.0f;

  player_init(&(game->player), 1, game->window->renderer.renderer_settings.height);

  struct TextureSettings sprite_texture_settings = {FILTER_NEAREST, MODE_CLAMP_TO_EDGE, FORMAT_R8G8B8A8_UNORM, true, true};
  texture_manager_init(&(game->texture_manager), &(mana->api.api_common));
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), &sprite_texture_settings, L"/textures/spritesheet.png");
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), &sprite_texture_settings, L"/textures/water.png");
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), &sprite_texture_settings, L"/textures/map.png");
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), &sprite_texture_settings, L"/textures/mario.png");
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), &sprite_texture_settings, L"/textures/whispy.png");
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), &sprite_texture_settings, L"/textures/track.png");
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), &sprite_texture_settings, L"/textures/startfinish.png");
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), &sprite_texture_settings, L"/textures/fence.png");

  sprite_manager_init(&(game->sprite_manager), &(game->texture_manager), &(mana->api.api_common), window->renderer.renderer_settings.width, window->renderer.renderer_settings.height, window->swap_chain->swap_chain_common.supersample_scale, &(window->gbuffer->gbuffer_common), window->renderer.renderer_settings.msaa_samples, 128);

  game->mario_speed = 0.0f;

  game->track = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), L"/textures/track.png");
  game->track->sprite_common.position = (vec3){.x = 0, .y = 0.0f, .z = 0};
  game->track->sprite_common.scale = (vec3){.x = 10.0f, .y = 10.0f, .z = 0.0f};

  game->start = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), L"/textures/startfinish.png");
  game->start->sprite_common.position = (vec3){.x = 0, .y = 90.0f, .z = 0.01f};
  game->start->sprite_common.scale = (vec3){.x = 10.0f, .y = 10.0f, .z = 0.0f};

  game->finish = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), L"/textures/startfinish.png");
  game->finish->sprite_common.position = (vec3){.x = 0, .y = -90.0f, .z = 0.01f};
  game->finish->sprite_common.scale = (vec3){.x = 10.0f, .y = 10.0f, .z = 0.0f};

  game->fence = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), L"/textures/fence.png");
  game->fence->sprite_common.position = (vec3){.x = 0, .y = 0.0f, .z = 2.5f};
  game->fence->sprite_common.scale = (vec3){.x = 5.0f, .y = 5.0f, .z = 0.0f};
  mat4 fence_rotation = mat4_rotate(MAT4_IDENTITY, -M_PI / 2, (vec3){.x = 0.5, .y = 0.0, .z = 0.0});
  fence_rotation = mat4_rotate(fence_rotation, M_PI, (vec3){.x = 0.0, .y = 1.0, .z = 0.0});
  game->fence->sprite_common.rotation = mat4_to_quaternion(fence_rotation);

  game->mario = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), L"/textures/mario.png");
  game->mario_position = (vec3){.x = 0.0f, .y = 95.0f, .z = 0.75};
  game->mario->sprite_common.position = game->mario_position;
  game->mario->sprite_common.scale = (vec3){.x = 5.0f, .y = 5.0f, .z = 0.0f};
  mat4 mario_rotation = mat4_rotate(MAT4_IDENTITY, -M_PI / 2, (vec3){.x = 0.5, .y = 0.0, .z = 0.0});
  // mario_rotation = mat4_rotate(mario_rotation, M_PI / 2, (vec3){.x = 0.0, .y = 1.0, .z = 0.0});
  game->mario->sprite_common.rotation = mat4_to_quaternion(mario_rotation);

  ///////////////////////////////////////

  WSADATA wsa;
  struct sockaddr_in server;

  // Initialize Winsock
  if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
    printf("WSAStartup failed\n");
    return;
  }

  game->sock = socket(AF_INET, SOCK_STREAM, 0);
  if (game->sock == INVALID_SOCKET) {
    printf("Socket creation failed\n");
    WSACleanup();
    return;
  }

  server.sin_family = AF_INET;
  server.sin_port = htons(5000);
  inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

  if (connect(game->sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
    printf("Connection failed\n");
    closesocket(game->sock);
    WSACleanup();
    return;
  }

  ///////////////////////////////////////

  // game->water = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), L"/textures/water.png");
  //// game->sprite->sprite_common.position = (vec3){.x = 0, .y = 15.0f, .z = 0};
  // game->water->sprite_common.position = (vec3){.x = 0, .y = 0.0f, .z = -10};
  // game->water->sprite_common.scale = (vec3){.x = 30.0f, .y = 30.0f, .z = 0.0f};
  // mat4 water_rotation = mat4_rotate(MAT4_IDENTITY, -M_PI, (vec3){.x = 1, .y = 0, .z = 0});
  ////  sprite_rotation.m00 = M_PI / 3.25f;
  // game->water->sprite_common.rotation = mat4_to_quaternion(water_rotation);
  //
  // game->map = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), L"/textures/map.png");
  //// game->sprite->sprite_common.position = (vec3){.x = 0, .y = 15.0f, .z = 0};
  // game->map->sprite_common.position = (vec3){.x = 0, .y = 0.0f, .z = 0};
  // game->map->sprite_common.scale = (vec3){.x = 20.0f, .y = 20.0f, .z = 0.0f};
  // mat4 map_rotation = mat4_rotate(MAT4_IDENTITY, -M_PI, (vec3){.x = 1, .y = 0, .z = 0});
  ////  sprite_rotation.m00 = M_PI / 3.25f;
  // game->map->sprite_common.rotation = mat4_to_quaternion(map_rotation);

  // for (int whispy_num = 0; whispy_num < 5; whispy_num++) {
  //   game->whispy[whispy_num] = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), L"/textures/whispy.png");
  //   if (whispy_num % 2 == 0)
  //     game->whispy[whispy_num]->sprite_common.position = (vec3){.x = 20.0f + whispy_num * 15.0f, .y = 20.0f + (whispy_num * 15.0f), .z = 3.0f};
  //   else
  //     game->whispy[whispy_num]->sprite_common.position = (vec3){.x = 20.0f + whispy_num * 15.0f, .y = 20.0f - (whispy_num * 5.0f), .z = 3.0f};
  //   game->whispy[whispy_num]->sprite_common.scale = (vec3){.x = 1.0f, .y = 1.0f, .z = 0.0f};
  //   mat4 whispy_rotation = mat4_rotate(MAT4_IDENTITY, -M_PI / 2, (vec3){.x = 0.5, .y = 0.0, .z = 0.0});
  //   whispy_rotation = mat4_rotate(whispy_rotation, M_PI, (vec3){.x = 0.0, .y = 1.0, .z = 0.0});
  //   game->whispy[whispy_num]->sprite_common.rotation = mat4_to_quaternion(whispy_rotation);
  // }

  // struct SpriteAnimation *sprite_animation = sprite_manager_add_sprite_animation(&(game->sprite_manager), &(mana->api.api_common), L"/textures/spritesheet.png", 4, 0.5f, 200);
  // sprite_animation->sprite_animation_common.position = (vec3){.x = 0, .y = 15.0f, .z = 0};
  // sprite_animation->sprite_animation_common.rotation = (quat){.data[0] = M_PI / 3.25f, .data[1] = 0, .data[2] = 0, .data[3] = 1.0f};
  // sprite_animation->sprite_animation_common.direction = SPRITE_ANIMATION_FORWARD;

  player_init(&(game->player), 1, game->window->renderer.renderer_settings.height);

  // game->player.player_controller.pos = (vec3d){.x = 0, .y = -500, .z = 5};
  // game->player.camera.look_at_azimuth += M_PI / 2;
}

void game_delete(struct Game* game, struct Mana* mana) {
  closesocket(game->sock);
  WSACleanup();

  struct APICommon* api_common = &(mana->api.api_common);

  // TODO: Throw this in a prepare delete function or something
  swap_chain_prepare_delete(game->window->swap_chain, api_common);

  sprite_manager_delete(&(game->sprite_manager), api_common);

  texture_manager_delete(&(game->texture_manager), api_common);
}

void game_update(struct Game* game, struct Mana* mana, double delta_time) {
  struct APICommon* api_common = &(mana->api.api_common);
  struct Window* window = game->window;
  struct InputManager* input_manager = &window->input_manager;

  // Note: Potential-based reward shaping
  float reward_func = ((-game->mario_position.y + 100.0f) / 2.0f) - (fabs(game->mario_position.x) / 5.0f);
  float reward = reward_func - game->previous_reward;
  game->previous_reward = reward_func;

  bool done = false;

  game->timer++;
  if (game->timer > 8640) {
    done = true;
  }

  if (reward_func > 100.0f)
    done = true;

  struct Packet {
    float x;
    float y;
    float reward;
    int done;
  };

  struct Packet packet;

  packet.x = game->mario_position.x;
  packet.y = game->mario_position.y;
  packet.reward = reward;
  packet.done = done;

  send(game->sock, (char*)&packet, sizeof(packet), 0);

  float action[2] = {0};
  recv_all(game->sock, (char*)action, sizeof(action));

  // printf("Action: %f %f\n", action[0], action[1]);

  float threshold = 0.3f;

  bool left = action[0] < -threshold;
  bool right = action[0] > threshold;
  bool backward = action[1] < -threshold;
  bool forward = action[1] > threshold;

  printf("Reward: %f\n Cummulate reward: %f\n", reward, reward_func);

  // printf("Mario position: %f %f\n", game->mario_position.x, (-game->mario_position.y + 100.0f) / 2.0f);

  // game->mario_position.y -= 1.5f * delta_time;
  game->mario->sprite_common.position = game->mario_position;
  // game->mario->sprite_common.rotation = quaternion_rotate_by_vector((quat){.data[0] = 0, .data[1] = game->mario_rotation, .data[2] = 0, .data[3] = 1.0f}, (vec3){.x = 0, .y = 1, .z = 0});
  game->player.look_at_pos = (vec3d){.x = game->mario_position.x, .y = game->mario_position.y, .z = game->mario_position.z};
  // game->player.camera.look_at_azimuth = game->mario_drive_rotation;

  float move_speed = 30.0f;
  float rotation_speed = 1.5f - game->mario_speed / 50.0f;

  if (true) {
    if (left) {
      float angle = -rotation_speed * delta_time;
      mat4 mario_rotation = quaternion_to_mat4(game->mario->sprite_common.rotation);
      mario_rotation = mat4_rotate(mario_rotation, angle, (vec3){.x = 0.0, .y = 1.0, .z = 0.0});
      game->mario->sprite_common.rotation = mat4_to_quaternion(mario_rotation);

      game->player.camera.look_at_azimuth -= angle;
    }
    if (right) {
      float angle = rotation_speed * delta_time;
      mat4 mario_rotation = quaternion_to_mat4(game->mario->sprite_common.rotation);
      mario_rotation = mat4_rotate(mario_rotation, angle, (vec3){.x = 0.0, .y = 1.0, .z = 0.0});
      game->mario->sprite_common.rotation = mat4_to_quaternion(mario_rotation);

      game->player.camera.look_at_azimuth -= angle;
    }
    if (forward) {
      game->mario_speed += move_speed * delta_time;
    }
    if (backward) {
      game->mario_speed -= move_speed * delta_time;
    }
  }

  vec3 forward_vel = {.x = cosf(game->player.camera.look_at_azimuth), .y = sinf(game->player.camera.look_at_azimuth), .z = 0.0f};

  for (size_t i = 0; i < input_manager->controllers.size; i++) {
    struct Controller* controller = array_list_get(&(input_manager->controllers), i);
    if (controller->controller_common.controller_type == CONTROLLER_KEYBOARD_MOUSE) {
      struct KeyboardMouseController* keyboard_mouse_controller = &(controller->controller_common.keyboard_mouse_controller);
      if (keyboard_mouse_controller->keys[KEY_W].state == PRESSED) {
        game->mario_speed += move_speed * delta_time;
      }
      if (keyboard_mouse_controller->keys[KEY_S].state == PRESSED) {
        game->mario_speed -= move_speed * delta_time;
      }
      if (keyboard_mouse_controller->keys[KEY_A].state == PRESSED) {
        float angle = -rotation_speed * delta_time;
        mat4 mario_rotation = quaternion_to_mat4(game->mario->sprite_common.rotation);
        mario_rotation = mat4_rotate(mario_rotation, angle, (vec3){.x = 0.0, .y = 1.0, .z = 0.0});
        game->mario->sprite_common.rotation = mat4_to_quaternion(mario_rotation);

        game->player.camera.look_at_azimuth -= angle;
      }
      if (keyboard_mouse_controller->keys[KEY_D].state == PRESSED) {
        float angle = rotation_speed * delta_time;
        mat4 mario_rotation = quaternion_to_mat4(game->mario->sprite_common.rotation);
        mario_rotation = mat4_rotate(mario_rotation, angle, (vec3){.x = 0.0, .y = 1.0, .z = 0.0});
        game->mario->sprite_common.rotation = mat4_to_quaternion(mario_rotation);

        game->player.camera.look_at_azimuth -= angle;
      }
    }
  }

  game->mario_position.x += forward_vel.x * game->mario_speed * delta_time;
  game->mario_position.y += forward_vel.y * game->mario_speed * delta_time;

  game->mario_speed *= 0.999f * (1.0f - delta_time);

  // for (int whispy_num = 0; whispy_num < 5; whispy_num++) {
  //   struct Sprite* whispy = game->whispy[whispy_num];
  //
  //  // Make sprite face the camera (Y-axis billboard)
  //  float angle = game->player.camera.look_at_azimuth;
  //  mat4 rotation = mat4_rotate(MAT4_IDENTITY, -M_PI / 2, (vec3){.x = 0.5, .y = 0.0, .z = 0.0});
  //  rotation = mat4_rotate(rotation, -angle + M_PI / 2, (vec3){.x = 0.0f, .y = 1.0f, .z = 0.0f});
  //  whispy->sprite_common.rotation = mat4_to_quaternion(rotation);
  //}
  //
  // if (input_manager->keys[KEY_ESC].state == PRESSED)
  //   window->should_close = true;
  //
  // bool mouse_locked_left = false;
  // bool mouse_locked_right = false;
  // if (input_manager_in_window(&(window->input_manager), &(window->surface))) {
  //  mouse_locked_left = (input_manager->keys[MOUSE_LEFT].state == PRESSED) ? true : false;
  //  mouse_locked_right = (input_manager->keys[MOUSE_RIGHT].state == PRESSED) ? true : false;
  //}
  // camera_update_parameters_from_camera(&(game->camera));
  // if (game->camera.camera_state == CAMERA_FLY) {
  //  if (mouse_locked_right) {
  //    input_manager_show_cursor(input_manager, false);
  //    input_manager_lock_cursor(input_manager, &(window->surface), true);
  //  } else {
  //    input_manager_show_cursor(input_manager, true);
  //    input_manager_lock_cursor(input_manager, &(window->surface), false);
  //  }
  //
  //  if (input_manager->keys[KEY_SHIFT].state == PRESSED)
  //    ;
  //  if (input_manager->keys[KEY_W].state == PRESSED)
  //    camera_fly_move_forward(&(game->camera), delta_time);
  //  if (input_manager->keys[KEY_S].state == PRESSED)
  //    camera_fly_move_backward(&(game->camera), delta_time);
  //  if (input_manager->keys[KEY_A].state == PRESSED)
  //    camera_fly_move_left(&(game->camera), delta_time);
  //  if (input_manager->keys[KEY_D].state == PRESSED)
  //    camera_fly_move_right(&(game->camera), delta_time);
  //  if (input_manager->keys[KEY_E].state == PRESSED)
  //    camera_fly_move_up(&(game->camera), delta_time);
  //  if (input_manager->keys[KEY_Q].state == PRESSED)
  //    camera_fly_move_down(&(game->camera), delta_time);
  //  if (input_manager->keys[KEY_Z].state == PRESSED)
  //    camera_fly_roll_left(&(game->camera), delta_time);
  //  if (input_manager->keys[KEY_X].state == PRESSED)
  //    camera_fly_roll_right(&(game->camera), delta_time);
  //
  //  game->camera.fly_zoom += input_manager->mouse_wheel;
  //  input_manager->mouse_wheel = 0.0;
  //
  //  // TODO: Make sensitivity a setting 2.0 is hardcoded
  //  double mouse_sensitivity = 2.0;
  //  if (mouse_locked_right)
  //    camera_rotate(&(game->camera), (int32_t)(input_manager->mouse_x_pos_diff * mouse_sensitivity), (int32_t)(input_manager->mouse_y_pos_diff * mouse_sensitivity), (int32_t)(game->window->renderer.renderer_settings.width), (int32_t)(game->window->renderer.renderer_settings.height));
  //}
  // if (game->camera.camera_state == CAMERA_LOOK_AT) {
  //  if (mouse_locked_left || mouse_locked_right) {
  //    input_manager_show_cursor(input_manager, false);
  //    input_manager_lock_cursor(input_manager, &(window->surface), true);
  //  } else {
  //    input_manager_show_cursor(input_manager, true);
  //    input_manager_lock_cursor(input_manager, &(window->surface), false);
  //  }
  //
  //  if (mouse_locked_left)
  //    camera_rotate(&(game->camera), (int32_t)(input_manager->mouse_x_pos_diff), (int32_t)(input_manager->mouse_y_pos_diff), (int32_t)(game->window->renderer.renderer_settings.width), (int32_t)(game->window->renderer.renderer_settings.height));
  //  if (mouse_locked_right)
  //    camera_look_at_zoom(&(game->camera), input_manager->mouse_x_pos_diff, game->window->renderer.renderer_settings.height);
  //}
  // camera_update_camera_from_parameters(&(game->camera));
  // camera_update(&(game->camera));

  // game->sprite->sprite_common.rotation = quaternion_add((quat){.data[0] = game->sprite->sprite_common.rotation.x, .data[1] = game->sprite->sprite_common.rotation.y, .data[2] = game->sprite->sprite_common.rotation.z, .data[3] = game->sprite->sprite_common.rotation.w}, (quat){.data[0] = 0, .data[1] = 0, .data[2] = delta_time, .data[3] = 1.0f});

  player_update(&(game->player), input_manager_get_controller_actions(input_manager), input_manager_get_controller_action_list_length(input_manager));

  if (done) {
    game->timer = 0;
    game->mario_position = (vec3){.x = 0.0f, .y = 95.0f, .z = 0.75};
    game->mario_speed = 0.0f;
    game->player.camera.look_at_azimuth = -M_PI / 2;
    game->previous_reward = 0.0f;
  }

  window->gbuffer->gbuffer_common.projection_matrix = camera_get_projection_matrix(&(game->player.camera), window);
  window->gbuffer->gbuffer_common.view_matrix = camera_get_view_matrix(&(game->player.camera));

  // Note: Wait for window to say it's ready to be resized
  if (!IsIconic(window->surface.hwnd)) {
    if (window->should_resize) {
      renderer_wait_for_device(&(window->renderer), window->api_common);

      sprite_manager_resize(&(game->sprite_manager), api_common, window->renderer.renderer_settings.width, window->renderer.renderer_settings.height, window->renderer.renderer_settings.supersample_scale);

      window->should_resize = false;
    }

    sprite_manager_update(&(game->sprite_manager), delta_time);
    sprite_manager_update_uniforms(&(game->sprite_manager), api_common, &(window->gbuffer->gbuffer_common));

    // GBuffer, only texture that needs to be multisampled
    gbuffer_start(window->gbuffer, &(window->swap_chain->swap_chain_common), window->renderer.renderer_settings.msaa_samples);

    // Transparent sprites
    sprite_manager_render(&(game->sprite_manager), &(window->gbuffer->gbuffer_common));

    gbuffer_stop(window->gbuffer, api_common, window->renderer.renderer_settings.msaa_samples);

    // Note: Only need to blit gbuffer to post process texture once then ping pong with post processes themselves
    // Note: My renderer is forward so the color texture has the lighting applied to it
    post_process_resolve_render(window->post_process, api_common, window->gbuffer, &(window->swap_chain->swap_chain_common));
    // Do post process effects
    // Render GUI
    // Final blit to swap chain
    swap_chain_blit_render(window->swap_chain, &(window->post_process->post_process_common));
  }
}
