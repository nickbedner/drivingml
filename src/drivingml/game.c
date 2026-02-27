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

  player_init(&(game->player), 1, game->window->renderer.renderer_settings.height);
  game->car_heading = M_PI / 2.0f;  // facing down -Y

  game->previous_reward = 0.0f;

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
  // Build same rotation as runtime
  mat4 mario_rotation = MAT4_IDENTITY;

  mario_rotation = mat4_rotate(
      mario_rotation,
      -M_PI / 2,
      (vec3){0.5f, 0.0f, 0.0f});

  mario_rotation = mat4_rotate(
      mario_rotation,
      -game->car_heading + M_PI / 2,
      (vec3){0.0f, 1.0f, 0.0f});
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

  game->last_action[0] = 0.0f;
  game->last_action[1] = 0.0f;

  game->timer = 0;

  game->prev_y = game->mario_position.y;

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

  bool done = false;

  // Delta time clamping, good for ML but not so good for game?
  if (delta_time > 0.05)
    delta_time = 0.05;

  // Hardcoded to allow for 30 seconds
  game->timer++;
  // if (game->timer > 4320) {
  if (game->timer > 1800) {
    printf("Episode timed out\n");
    done = true;
  }

  float steer = game->last_action[0];
  float throttle = game->last_action[1];

  if (steer > 1.0f)
    steer = 1.0f;
  if (steer < -1.0f)
    steer = -1.0f;
  if (throttle > 1.0f)
    throttle = 1.0f;
  if (throttle < -1.0f)
    throttle = -1.0f;

  // printf("Steer: %f, Throttle: %f\n", steer, throttle);

  float move_speed = 30.0f;
  float rotation_speed = 1.5f;  // - game->mario_speed / 50.0f;

  for (size_t i = 0; i < input_manager->controllers.size; i++) {
    struct Controller* controller = array_list_get(&(input_manager->controllers), i);
    if (controller->controller_common.controller_type == CONTROLLER_KEYBOARD_MOUSE) {
      struct KeyboardMouseController* keyboard_mouse_controller = &(controller->controller_common.keyboard_mouse_controller);
      if (keyboard_mouse_controller->keys[KEY_W].state == PRESSED) {
        throttle = 1.0f;
      }
      if (keyboard_mouse_controller->keys[KEY_S].state == PRESSED) {
        throttle = -1.0f;
      }
      if (keyboard_mouse_controller->keys[KEY_A].state == PRESSED) {
        steer = -1.0f;
      }
      if (keyboard_mouse_controller->keys[KEY_D].state == PRESSED) {
        steer = 1.0f;
      }
    }
  }

  float angle = -rotation_speed * delta_time * steer;

  // Update car heading
  game->car_heading += angle;
  game->player.camera.look_at_azimuth = game->car_heading + M_PI;

  // Update sprite rotation from heading
  // Start from identity
  mat4 mario_rotation = MAT4_IDENTITY;

  // 1) Tilt upright (same as original working code)
  mario_rotation = mat4_rotate(
      mario_rotation,
      -M_PI / 2,
      (vec3){0.5f, 0.0f, 0.0f});

  // 2) Apply heading rotation
  mario_rotation = mat4_rotate(mario_rotation, -game->car_heading + M_PI / 2, (vec3){0.0f, 1.0f, 0.0f});
  game->mario->sprite_common.rotation = mat4_to_quaternion(mario_rotation);
  game->mario_speed += throttle * move_speed * delta_time;

  // Clamp speed (important!)
  if (game->mario_speed > 50.0f)
    game->mario_speed = 50.0f;
  if (game->mario_speed < -20.0f)
    game->mario_speed = -20.0f;

  // Movement direction
  float heading = game->car_heading;

  vec3 forward_vel = {-cosf(heading), -sinf(heading), 0.0f};

  game->mario_position.x += forward_vel.x * game->mario_speed * delta_time;
  game->mario_position.y += forward_vel.y * game->mario_speed * delta_time;

  // game->mario_speed *= 0.999f * (1.0f - delta_time);

  float damping = 2.0f;  // higher = stronger friction
  game->mario_speed *= expf(-damping * delta_time);

  // game->mario_position.y -= 1.5f * delta_time;
  game->mario->sprite_common.position = game->mario_position;
  // game->mario->sprite_common.rotation = quaternion_rotate_by_vector((quat){.data[0] = 0, .data[1] = game->mario_rotation, .data[2] = 0, .data[3] = 1.0f}, (vec3){.x = 0, .y = 1, .z = 0});
  game->player.look_at_pos = (vec3d){.x = game->mario_position.x, .y = game->mario_position.y, .z = game->mario_position.z};
  // game->player.camera.look_at_azimuth = game->mario_drive_rotation;

  float reward = 0.0f;

  const float track_half_width = 25.0f;
  const float forward_limit = -100.0f;

  // 1️⃣ Reward forward velocity directly
  float forward_speed = -sinf(game->car_heading) * game->mario_speed;
  reward += 0.5f * fmaxf(0.0f, forward_speed);

  // 2️⃣ Penalize being far from center
  float xnorm = fabsf(game->mario_position.x) / track_half_width;
  reward -= 1.5f * xnorm * xnorm;

  // 3️⃣ Penalize sharp turning (smoothness)
  reward -= 0.2f * steer * steer;

  // 4️⃣ Small time penalty (encourage faster finish)
  reward -= 0.01f;

  // Note: Potential-based reward shaping
  // Small time penalty
  // Reward shaping (prevents "spin + drift + crash" exploit)
  // float reward = 0.0f;
  //
  // const float track_half_width = 25.0f;
  // const float forward_limit = -100.0f;
  // const float backward_limit = 100.0f;
  //
  //// True forward progress = decrease in Y
  // float dy = game->prev_y - game->mario_position.y;
  // game->prev_y = game->mario_position.y;
  //
  //// 1) Only reward progress when facing the correct way (-Y)
  //// heading is game->car_heading
  // float alignment = sinf(heading);         // ~1 when facing -Y, ~-1 when facing +Y
  // float aligned = fmaxf(0.0f, alignment);  // clamp to [0,1]
  // reward += 0.25f * dy * aligned;
  //
  // reward += 0.02f * aligned;                  // small shaping for facing correct way
  // reward -= 0.02f * fmaxf(0.0f, -alignment);  // small penalty for facing backward
  //
  //// 2) Strong lane keeping penalty that ramps up near edges
  // float xnorm = fabsf(game->mario_position.x) / track_half_width;  // 0..>1
  // reward -= 2.0f * xnorm * xnorm * xnorm;                          // cubic
  //
  //// 3) Penalize violent steering / spinning
  // reward -= 0.05f * fabsf(steer);  // discourages full-lock steering
  // reward -= 0.50f * fabsf(angle);  // discourages high yaw-rate (spinning)
  //
  //// 4) Penalize reversing more strongly
  // if (game->mario_speed < 0.0f)
  //   reward -= 0.20f * (-game->mario_speed);
  //
  //// 5) Small living cost (less incentive to stall)
  // reward -= 0.005f;

  // if (fabsf(game->mario_position.x) > track_half_width) {
  //   done = true;
  //   reward -= 200.0f;
  // }

  if (fabsf(game->mario_position.x) > track_half_width) {
    float side = (game->mario_position.x > 0.0f) ? 1.0f : -1.0f;

    // Clamp to boundary
    game->mario_position.x = side * track_half_width;

    // Kill outward velocity component
    if ((side > 0.0f && game->mario_speed * -cosf(game->car_heading) > 0.0f) ||
        (side < 0.0f && game->mario_speed * -cosf(game->car_heading) < 0.0f)) {
      game->mario_speed = 0.0f;
    }

    // Continuous wall penalty (scaled by speed so slamming is worse)
    reward -= 5.0f + 0.1f * fabsf(game->mario_speed);
  }

  const float backward_limit = 100.0f;

  if (game->mario_position.y < forward_limit) {
    done = true;
    reward += 200.0f;  // strong success bonus
  }

  if (game->mario_position.y > backward_limit) {
    reward -= 5.0f;
    game->mario_position.y = backward_limit;
    if (game->mario_speed > 0)
      game->mario_speed = 0;
  }

  // if (game->mario_position.y < forward_limit) {
  //   done = true;
  //   if (alignment > 0.7f && game->mario_speed > 0.0f)
  //     reward += 50.0f;
  //   else
  //     reward -= 50.0f;
  // }

  // if (game->mario_position.y > backward_limit) {
  //   reward -= 5.0f;                                    // per frame penalty while behind start
  //   game->mario_position.y = backward_limit;           // clamp
  //   if (game->mario_speed > 0) game->mario_speed = 0;  // prevent pushing further back
  // }

  // if (game->mario_position.y > backward_limit) {
  //   done = true;
  //   reward -= 500.0f;
  // }

  struct Packet {
    float x;
    float y;
    float speed;
    float azimuth;  // radians
    float reward;
    int done;
  };

  struct Packet packet;
  packet.x = game->mario_position.x;
  packet.y = game->mario_position.y;
  packet.speed = game->mario_speed;
  packet.azimuth = heading;
  packet.reward = reward;
  packet.done = done;

  send(game->sock, (char*)&packet, sizeof(packet), 0);

  if (recv_all(game->sock, (char*)game->last_action, sizeof(game->last_action)) <= 0) {
    game->last_action[0] = 0.0f;
    game->last_action[1] = 0.0f;
  }

  player_update(&(game->player), input_manager_get_controller_actions(input_manager), input_manager_get_controller_action_list_length(input_manager));

  if (done) {
    game->timer = 0;
    game->mario_position = (vec3){0.0f, 95.0f, 0.75f};

    game->mario_speed = 0.0f;
    game->car_heading = M_PI / 2;
    mat4 mario_rotation = MAT4_IDENTITY;

    mario_rotation = mat4_rotate(
        mario_rotation,
        -M_PI / 2,
        (vec3){0.5f, 0.0f, 0.0f});

    mario_rotation = mat4_rotate(
        mario_rotation,
        -game->car_heading + M_PI / 2,
        (vec3){0.0f, 1.0f, 0.0f});

    game->mario->sprite_common.rotation =
        mat4_to_quaternion(mario_rotation);
    // game->player.camera.look_at_azimuth = game->car_heading;

    game->last_action[0] = 0.0f;
    game->last_action[1] = 0.0f;
    game->prev_y = game->mario_position.y;
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
