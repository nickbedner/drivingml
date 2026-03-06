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

static inline void place_marker(struct Sprite* marker, float x, float y) {
  marker->sprite_common.position = (vec3){.x = x, .y = y, .z = 2.35f};
  marker->sprite_common.scale = (vec3){.x = 1.0f, .y = 1.0f, .z = 0.0f};
  mat4 marker_rotation_0 = mat4_rotate(MAT4_IDENTITY, -M_PI / 2, (vec3){.x = 0.5, .y = 0.0, .z = 0.0});
  marker_rotation_0 = mat4_rotate(marker_rotation_0, M_PI, (vec3){.x = 0.0, .y = 1.0, .z = 0.0});
  marker->sprite_common.rotation = mat4_to_quaternion(marker_rotation_0);
}

void game_init(struct Game* game, struct Mana* mana, struct Window* window) {
  game->window = window;

  player_init(&(game->player), 1, game->window->renderer.renderer_settings.height);

  game->previous_reward = 0.0f;

  struct TextureSettings sprite_texture_settings = {FILTER_NEAREST, MODE_CLAMP_TO_EDGE, FORMAT_R8G8B8A8_UNORM, true, true};
  texture_manager_init(&(game->texture_manager), &(mana->api.api_common));
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), &sprite_texture_settings, L"/textures/spritesheet.png");
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), &sprite_texture_settings, L"/textures/water.png");
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), &sprite_texture_settings, L"/textures/map.png");
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), &sprite_texture_settings, L"/textures/rb.png");
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), &sprite_texture_settings, L"/textures/whispy.png");
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), &sprite_texture_settings, L"/textures/track.png");
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), &sprite_texture_settings, L"/textures/circuit.png");
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), &sprite_texture_settings, L"/textures/startfinish.png");
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), &sprite_texture_settings, L"/textures/fence.png");
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), &sprite_texture_settings, L"/textures/marker.png");

  sprite_manager_init(&(game->sprite_manager), &(game->texture_manager), &(mana->api.api_common), window->renderer.renderer_settings.width, window->renderer.renderer_settings.height, window->swap_chain->swap_chain_common.supersample_scale, &(window->gbuffer->gbuffer_common), window->renderer.renderer_settings.msaa_samples, 128);

  game->mario_speed = 0.0f;

  game->track = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), L"/textures/map.png");
  game->track->sprite_common.position = (vec3){.x = 0, .y = 0.0f, .z = 0};
  game->track->sprite_common.scale = (vec3){.x = 25.0f, .y = 25.0f, .z = 0.0f};
  mat4 track_rotation = mat4_rotate(MAT4_IDENTITY, M_PI, (vec3){.x = 0.0, .y = 0.5, .z = 0.0});
  game->track->sprite_common.rotation = mat4_to_quaternion(track_rotation);

  game->start = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), L"/textures/startfinish.png");
  game->start->sprite_common.position = (vec3){.x = 0, .y = 100.0f, .z = 0.01f};
  game->start->sprite_common.scale = (vec3){.x = 10.0f, .y = 10.0f, .z = 0.0f};
  mat4 start_rotation = mat4_rotate(MAT4_IDENTITY, M_PI, (vec3){.x = 0.0, .y = 0.5, .z = 0.0});
  start_rotation = mat4_rotate(start_rotation, -M_PI / 2, (vec3){.x = 0.0, .y = 0.0, .z = 0.5});
  game->start->sprite_common.rotation = mat4_to_quaternion(start_rotation);

  game->fence = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), L"/textures/fence.png");
  game->fence->sprite_common.position = (vec3){.x = 0, .y = 0.0f, .z = 2.5f};
  game->fence->sprite_common.scale = (vec3){.x = 5.0f, .y = 5.0f, .z = 0.0f};
  mat4 fence_rotation = mat4_rotate(MAT4_IDENTITY, -M_PI / 2, (vec3){.x = 0.5, .y = 0.0, .z = 0.0});
  fence_rotation = mat4_rotate(fence_rotation, M_PI, (vec3){.x = 0.0, .y = 1.0, .z = 0.0});
  game->fence->sprite_common.rotation = mat4_to_quaternion(fence_rotation);

  game->mario = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), L"/textures/rb.png");
  game->mario_position = (vec3){.x = 10.0f, .y = 95.0f, .z = 0.75};
  game->mario->sprite_common.position = game->mario_position;
  game->mario->sprite_common.scale = (vec3){.x = 5.0f, .y = 5.0f, .z = 0.0f};
  game->car_heading = 0.0f;  // M_PI / 2.0f;  // facing down -Y

  game->total_markers = 15;
  game->marker[0] = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), L"/textures/marker.png");
  place_marker(game->marker[0], -95.0f, 100.0f);
  game->marker[1] = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), L"/textures/marker.png");
  place_marker(game->marker[1], -95.0f, 50.0f);
  game->marker[2] = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), L"/textures/marker.png");
  place_marker(game->marker[2], -20.0f, 50.0f);
  game->marker[3] = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), L"/textures/marker.png");
  place_marker(game->marker[3], -95.0f, 20.0f);
  game->marker[4] = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), L"/textures/marker.png");
  place_marker(game->marker[4], -95.0f, -20.0f);
  game->marker[5] = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), L"/textures/marker.png");
  place_marker(game->marker[5], -25.0f, -20.0f);
  game->marker[6] = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), L"/textures/marker.png");
  place_marker(game->marker[6], -95.0f, -50.0f);
  game->marker[7] = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), L"/textures/marker.png");
  place_marker(game->marker[7], -95.0f, -90.0f);
  game->marker[8] = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), L"/textures/marker.png");
  place_marker(game->marker[8], 140.0f, -90.0f);
  game->marker[9] = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), L"/textures/marker.png");
  place_marker(game->marker[9], 130.0f, -30.0f);
  game->marker[10] = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), L"/textures/marker.png");
  place_marker(game->marker[10], 65.0f, -30.0f);
  game->marker[11] = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), L"/textures/marker.png");
  place_marker(game->marker[11], 50.0f, 20.0f);
  game->marker[12] = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), L"/textures/marker.png");
  place_marker(game->marker[12], 65.0f, 60.0f);
  game->marker[13] = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), L"/textures/marker.png");
  place_marker(game->marker[13], 110.0f, 65.0f);
  game->marker[14] = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), L"/textures/marker.png");
  place_marker(game->marker[14], 110.0f, 100.0f);
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

  game->last_action[0] = 0.0f;
  game->last_action[1] = 0.0f;

  game->timer = 0;

  game->prev_y = game->mario_position.y;

  game->current_marker = 0;
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

  // Hardcoded to episode length of 1 minute before timeout
  game->timer++;
  if (game->timer > 3600 && !EVAL_MODE) {
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
  mat4 mario_rotation = mat4_rotate(MAT4_IDENTITY, -M_PI / 2, (vec3){0.5f, 0.0f, 0.0f});
  mario_rotation = mat4_rotate(mario_rotation, -game->car_heading - M_PI / 2, (vec3){0.0f, 1.0f, 0.0f});
  game->mario->sprite_common.rotation = mat4_to_quaternion(mario_rotation);
  game->mario_speed += throttle * move_speed * delta_time;

  // Clamp speed
  if (game->mario_speed > 50.0f)
    game->mario_speed = 50.0f;
  if (game->mario_speed < -20.0f)
    game->mario_speed = -20.0f;

  // Movement + Progress-Based Reward
  float heading = game->car_heading;
  vec3 forward_vel = {-cosf(heading), -sinf(heading), 0.0f};

  // Current marker position
  vec3 marker_pos = game->marker[game->current_marker]->sprite_common.position;

  // Distance BEFORE movement
  float dx_before = marker_pos.x - game->mario_position.x;
  float dy_before = marker_pos.y - game->mario_position.y;
  float dist_before = sqrtf(dx_before * dx_before + dy_before * dy_before);

  //  Move car
  game->mario_position.x += forward_vel.x * game->mario_speed * delta_time;
  game->mario_position.y += forward_vel.y * game->mario_speed * delta_time;

  // Apply damping
  float damping = 2.0f;
  game->mario_speed *= expf(-damping * delta_time);

  // Update sprite + camera
  game->mario->sprite_common.position = game->mario_position;
  game->player.look_at_pos = (vec3d){.x = game->mario_position.x, .y = game->mario_position.y, .z = game->mario_position.z};

  //  Distance AFTER movement
  float dx_after = marker_pos.x - game->mario_position.x;
  float dy_after = marker_pos.y - game->mario_position.y;
  float dist_after = sqrtf(dx_after * dx_after + dy_after * dy_after);

  // Reward Calculation
  float reward = 0.0f;

  // Main dense signal: reward distance reduction
  float progress = dist_before - dist_after;
  reward += 0.1f * progress;

  //  Checkpoint reward
  const float checkpoint_radius = 10.0f;

  if (dist_after < checkpoint_radius) {
    reward += 2.0f;  // checkpoint bonus

    game->current_marker++;

    if (game->current_marker >= game->total_markers) {
      game->current_marker = 0;
      reward += 5.0f;  // lap bonus
      // done = true;
      game->timer = 0;
    }
  }

  // Penalize reversing
  if (game->mario_speed < 0.0f) {
    reward -= 0.05f;
  }

  // Small time penalty
  reward -= 0.005f;

  // Steering penalty
  reward -= 0.01f * steer * steer;
  reward -= 0.02f * fabsf(angle);

  vec3 next_marker = game->marker[game->current_marker]->sprite_common.position;

  // World delta
  float dxw = next_marker.x - game->mario_position.x;
  float dyw = next_marker.y - game->mario_position.y;

  // Car forward
  float fx = -cosf(game->car_heading);
  float fy = -sinf(game->car_heading);

  // Car right
  float rx = fy;
  float ry = -fx;

  // Project to car frame
  float forward_err = dxw * fx + dyw * fy;
  float right_err = dxw * rx + dyw * ry;

  // Better normalization for 100x100 map
  const float norm = 150.0f;

  struct Packet {
    float dx;
    float dy;
    float speed;
    float azimuth;
    float reward;
    int done;
  };

  struct Packet packet;
  // Normalize to roughly [-1,1]
  packet.dx = forward_err / norm;
  packet.dy = right_err / norm;
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

    game->mario_speed = 0.0f;
    game->mario_position = (vec3){.x = 10.0f, .y = 95.0f, .z = 0.75};
    game->mario->sprite_common.position = game->mario_position;
    game->mario->sprite_common.scale = (vec3){.x = 5.0f, .y = 5.0f, .z = 0.0f};
    game->car_heading = 0.0f;  // M_PI / 2.0f;  // facing down -Y
    mat4 mario_rotation = mat4_rotate(MAT4_IDENTITY, -M_PI / 2, (vec3){0.5f, 0.0f, 0.0f});
    mario_rotation = mat4_rotate(mario_rotation, -game->car_heading + M_PI / 2, (vec3){0.0f, 1.0f, 0.0f});
    game->mario->sprite_common.rotation = mat4_to_quaternion(mario_rotation);

    game->last_action[0] = 0.0f;
    game->last_action[1] = 0.0f;
    game->prev_y = game->mario_position.y;

    game->current_marker = 0;
  }

  // Make this always facing toward the camera
  for (int marker_num = 0; marker_num < game->total_markers; marker_num++) {
    mat4 marker_rotation = mat4_rotate(MAT4_IDENTITY, -M_PI / 2, (vec3){.x = 0.5, .y = 0.0, .z = 0.0});
    marker_rotation = mat4_rotate(marker_rotation, M_PI / 2, (vec3){.x = 0.0, .y = 1.0, .z = 0.0});
    game->marker[marker_num]->sprite_common.rotation = mat4_to_quaternion(mat4_rotate(marker_rotation, -game->player.camera.look_at_azimuth, (vec3){0.0f, 1.0f, 0.0f}));
    if (EVAL_MODE)
      game->marker[marker_num]->sprite_common.position.z = -10000.0f;
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
    vec3d cam_pos = camera_get_pos(&game->player.camera);
    vec3d cam_forward = vec3d_normalise(vec3d_sub(game->player.camera.target, game->player.camera.eye));
    vec4d sort_key;
    sort_key.x = cam_forward.x;
    sort_key.y = cam_forward.y;
    sort_key.z = cam_forward.z;
    sort_key.w = -vec3d_dot(cam_forward, cam_pos);
    sprite_manager_render(&(game->sprite_manager), &(window->gbuffer->gbuffer_common), sort_key);

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
