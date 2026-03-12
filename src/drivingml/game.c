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

static void load_map_from_xml(struct Game* game, struct Mana* mana, const char* xml_path, const char* map_name) {
  struct XmlNode* root = xml_parser_load_xml_file((char*)xml_path);
  if (!root)
    return;

  struct XmlNode* map_node = xml_node_get_child_with_attribute(root, "map", "name", (char*)map_name);

  if (!map_node) {
    xml_parser_delete(root);
    return;
  }
  // Track
  struct XmlNode* track_node = xml_node_get_child(map_node, "track");
  if (!track_node) {
    xml_parser_delete(root);
    return;
  }

  char* tex = xml_node_get_attribute(track_node, "texture");
  char* sx = xml_node_get_attribute(track_node, "scale");
  char* px = xml_node_get_attribute(track_node, "x");
  char* py = xml_node_get_attribute(track_node, "y");
  if (tex) {
    wchar_t wtex[256];
    mbstowcs(wtex, tex, 256);

    game->track = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), wtex);

    float scale = sx ? (float)atof(sx) : 25.0f;
    float x = px ? (float)atof(px) : 0.0f;
    float y = py ? (float)atof(py) : 0.0f;

    game->track->sprite_common.position = (vec3){x, y, 0};
    game->track->sprite_common.scale = (vec3){scale, scale, 0};

    mat4 rot = mat4_rotate(MAT4_IDENTITY, M_PI, (vec3){0.0, 0.5, 0.0});
    game->track->sprite_common.rotation = mat4_to_quaternion(rot);
  }

  // Markers
  struct XmlNode* markers_node = xml_node_get_child(map_node, "markers");

  if (markers_node) {
    struct ArrayList* marker_list = xml_node_get_children(markers_node, "marker");

    if (marker_list) {
      size_t count = array_list_size(marker_list);
      game->total_markers = (int)count;

      for (size_t i = 0; i < count; i++) {
        struct XmlNode* marker = (struct XmlNode*)array_list_get(marker_list, i);

        char* x_str = xml_node_get_attribute(marker, "x");
        char* y_str = xml_node_get_attribute(marker, "y");

        if (!x_str || !y_str) continue;

        float x = (float)atof(x_str);
        float y = (float)atof(y_str);

        game->marker[i] = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), L"/textures/marker.png");
        place_marker(game->marker[i], x, y);
      }
    }
  }

  xml_parser_delete(root);
}

void game_init(struct Game* game, struct Mana* mana, struct Window* window) {
  game->window = window;

  player_init(&(game->player), 1, game->window->renderer.renderer_settings.height);

  game->previous_reward = 0.0f;

  struct TextureSettings sprite_texture_settings = {FILTER_NEAREST, MODE_CLAMP_TO_EDGE, FORMAT_R8G8B8A8_UNORM, MIP_GENERATE, 5, true};
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
  sprite_texture_settings = (struct TextureSettings){.filter_type = FILTER_LINEAR, .mode_type = MODE_REPEAT, .format_type = FORMAT_R8G8B8A8_UNORM, .mip_type = MIP_CUSTOM, .mip_count = 5, .premultiplied_alpha = true};
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), &sprite_texture_settings, L"/textures/waterm1.png");

  sprite_manager_init(&(game->sprite_manager), &(game->texture_manager), &(mana->api.api_common), window->renderer.renderer_settings.width, window->renderer.renderer_settings.height, window->swap_chain->swap_chain_common.supersample_scale, &(window->gbuffer->gbuffer_common), window->renderer.renderer_settings.msaa_samples, 128);

  wchar_t wpath[MAX_LENGTH_OF_PATH] = {0};
  swprintf(wpath, MAX_LENGTH_OF_PATH, L"%ls/maps.xml", mana->api.api_common.asset_directory);
  char path[MAX_LENGTH_OF_PATH] = {0};
  wcstombs(path, wpath, MAX_LENGTH_OF_PATH);
  load_map_from_xml(game, mana, path, "track1");

  game->fence = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), L"/textures/fence.png");
  game->fence->sprite_common.position = (vec3){.x = 0, .y = 0.0f, .z = 2.5f};
  game->fence->sprite_common.scale = (vec3){.x = 5.0f, .y = 5.0f, .z = 0.0f};
  mat4 fence_rotation = mat4_rotate(MAT4_IDENTITY, -M_PI / 2, (vec3){.x = 0.5, .y = 0.0, .z = 0.0});
  fence_rotation = mat4_rotate(fence_rotation, M_PI, (vec3){.x = 0.0, .y = 1.0, .z = 0.0});
  game->fence->sprite_common.rotation = mat4_to_quaternion(fence_rotation);

  // game->mario = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), L"/textures/rb.png");
  // game->mario_position = (vec3){.x = 10.0f, .y = 95.0f, .z = 0.75};
  // game->mario->sprite_common.position = game->mario_position;
  // game->mario->sprite_common.scale = (vec3){.x = 5.0f, .y = 5.0f, .z = 0.0f};
  // game->car_heading = 0.0f;  // M_PI / 2.0f;  // facing down -Y

  if (!EVAL_MODE) {
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
    }
  }

  game->timer = 0;
  game->start_timer = 0;

  if (EVAL_MODE)
    game->current_npcs += MAX_NPCS;
  else
    game->current_npcs += 1;
  for (int npc_num = 0; npc_num < game->current_npcs; npc_num++) {
    game->npcs[npc_num].sprite = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), L"/textures/rb.png");
    game->npcs[npc_num].speed = 0.0f;
    game->npcs[npc_num].position = (vec3){.x = 10.0f + (npc_num * 10), .y = 95.0f + ((npc_num % 4) * 3.0f), .z = 0.75};
    game->npcs[npc_num].sprite->sprite_common.position = game->npcs[npc_num].position;
    game->npcs[npc_num].sprite->sprite_common.scale = (vec3){.x = 5.0f, .y = 5.0f, .z = 0.0f};
    game->npcs[npc_num].heading = 0.0f;  // M_PI / 2.0f;  // facing down -Y
    game->npcs[npc_num].current_marker = 0;
    game->npcs[npc_num].last_action[0] = 0.0f;
    game->npcs[npc_num].last_action[1] = 0.0f;
    game->npcs[npc_num].prev_y = game->npcs[npc_num].position.y;
    load_ac_model("checkpoints/ac_weights.bin", &(game->npcs[npc_num].model));
  }

  water_shader_init(&(game->water_shader), &(mana->api.api_common), window->renderer.renderer_settings.width, window->renderer.renderer_settings.height, window->swap_chain->swap_chain_common.supersample_scale, &(window->gbuffer->gbuffer_common), window->renderer.renderer_settings.msaa_samples, 3);
  water_init(&(game->water), &(mana->api.api_common), &(game->water_shader.shader), texture_manager_get(game->sprite_manager.sprite_manager_common.texture_manager, L"/textures/waterm1.png"));
  game->water.water_common.position = (vec3){0.0f, 0.0f, -5.0f};
  game->water.water_common.scale = (vec3){1024.0f, 1024.0f, 1.0f};
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

  game->start_timer++;
  bool start = false;
  if (game->start_timer > 180)
    start = true;

  float move_speed = 30.0f;
  float rotation_speed = 1.5f;  // - game->mario_speed / 50.0f;

  for (int ai_num = 0; ai_num < game->current_npcs; ai_num++) {
    float steer = game->npcs[ai_num].last_action[0];
    float throttle = game->npcs[ai_num].last_action[1];

    if (steer > 1.0f)
      steer = 1.0f;
    if (steer < -1.0f)
      steer = -1.0f;
    if (throttle > 1.0f)
      throttle = 1.0f;
    if (throttle < -1.0f)
      throttle = -1.0f;

    // printf("Steer: %f, Throttle: %f\n", steer, throttle);

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
    game->npcs[ai_num].heading += angle;
    game->player.camera.look_at_azimuth = game->npcs[ai_num].heading + M_PI;
    mat4 mario_rotation = mat4_rotate(MAT4_IDENTITY, -M_PI / 2, (vec3){0.5f, 0.0f, 0.0f});
    mario_rotation = mat4_rotate(mario_rotation, -game->npcs[ai_num].heading - M_PI / 2, (vec3){0.0f, 1.0f, 0.0f});
    game->npcs[ai_num].sprite->sprite_common.rotation = mat4_to_quaternion(mario_rotation);
    game->npcs[ai_num].speed += throttle * move_speed * delta_time;

    // Clamp speed
    if (game->npcs[ai_num].speed > 50.0f)
      game->npcs[ai_num].speed = 50.0f;
    if (game->npcs[ai_num].speed < -20.0f)
      game->npcs[ai_num].speed = -20.0f;

    // Movement + progress based reward
    float heading = game->npcs[ai_num].heading;
    vec3 forward_vel = {-cosf(heading), -sinf(heading), 0.0f};

    // Current marker position
    vec3 marker_pos = game->marker[game->npcs[ai_num].current_marker]->sprite_common.position;

    // Distance BEFORE movement
    float dx_before = marker_pos.x - game->npcs[ai_num].position.x;
    float dy_before = marker_pos.y - game->npcs[ai_num].position.y;
    float dist_before = sqrtf(dx_before * dx_before + dy_before * dy_before);

    //  Move car
    game->npcs[ai_num].position.x += forward_vel.x * game->npcs[ai_num].speed * delta_time;
    game->npcs[ai_num].position.y += forward_vel.y * game->npcs[ai_num].speed * delta_time;

    // Apply damping
    float damping = 2.0f;
    game->npcs[ai_num].speed *= expf(-damping * delta_time);

    // Update sprite + camera
    game->npcs[ai_num].sprite->sprite_common.position = game->npcs[ai_num].position;
    game->player.look_at_pos = (vec3d){.x = game->npcs[ai_num].position.x, .y = game->npcs[ai_num].position.y, .z = game->npcs[ai_num].position.z};

    //  Distance AFTER movement
    float dx_after = marker_pos.x - game->npcs[ai_num].position.x;
    float dy_after = marker_pos.y - game->npcs[ai_num].position.y;
    float dist_after = sqrtf(dx_after * dx_after + dy_after * dy_after);

    // Reward Calculation
    float reward = 0.0f;

    // Main dense signal: reward distance reduction
    float progress = dist_before - dist_after;
    reward += 0.1f * progress;

    //  Checkpoint reward
    const float checkpoint_radius = 15.0f;

    if (dist_after < checkpoint_radius) {
      reward += 2.0f;  // checkpoint bonus

      game->npcs[ai_num].current_marker++;

      if (game->npcs[ai_num].current_marker >= game->total_markers) {
        game->npcs[ai_num].current_marker = 0;
        reward += 5.0f;  // lap bonus
        // done = true;
        game->timer = 0;
      }
    }

    // Penalize reversing
    if (game->npcs[ai_num].speed < 0.0f) {
      reward -= 0.05f;
    }

    // Small time penalty
    reward -= 0.005f;

    // Steering penalty
    reward -= 0.01f * steer * steer;
    reward -= 0.02f * fabsf(angle);

    vec3 next_marker = game->marker[game->npcs[ai_num].current_marker]->sprite_common.position;

    // World delta
    float dxw = next_marker.x - game->npcs[ai_num].position.x;
    float dyw = next_marker.y - game->npcs[ai_num].position.y;

    // Car forward
    float fx = -cosf(game->npcs[ai_num].heading);
    float fy = -sinf(game->npcs[ai_num].heading);

    // Car right
    float rx = fy;
    float ry = -fx;

    // Project to car frame
    float forward_err = dxw * fx + dyw * fy;
    float right_err = dxw * rx + dyw * ry;

    // Better normalization for 100x100 map
    const float norm = 150.0f;

    if (!EVAL_MODE) {
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
      packet.speed = game->npcs[ai_num].speed;
      packet.azimuth = heading;
      packet.reward = reward;
      packet.done = done;

      send(game->sock, (char*)&packet, sizeof(packet), 0);

      if (recv_all(game->sock, (char*)game->npcs[ai_num].last_action, sizeof(game->npcs[ai_num].last_action)) <= 0) {
        game->npcs[ai_num].last_action[0] = 0.0f;
        game->npcs[ai_num].last_action[1] = 0.0f;
      }

      if (done) {
        game->timer = 0;

        game->npcs[ai_num].speed = 0.0f;
        game->npcs[ai_num].position = (vec3){.x = 10.0f, .y = 95.0f, .z = 0.75};
        game->npcs[ai_num].sprite->sprite_common.position = game->npcs[ai_num].position;
        game->npcs[ai_num].sprite->sprite_common.scale = (vec3){.x = 5.0f, .y = 5.0f, .z = 0.0f};
        game->npcs[ai_num].heading = 0.0f;  // M_PI / 2.0f;  // facing down -Y
        mat4 mario_rotation = mat4_rotate(MAT4_IDENTITY, -M_PI / 2, (vec3){0.5f, 0.0f, 0.0f});
        mario_rotation = mat4_rotate(mario_rotation, -game->npcs[ai_num].heading + M_PI / 2, (vec3){0.0f, 1.0f, 0.0f});
        game->npcs[ai_num].sprite->sprite_common.rotation = mat4_to_quaternion(mario_rotation);

        game->npcs[ai_num].last_action[0] = 0.0f;
        game->npcs[ai_num].last_action[1] = 0.0f;
        game->npcs[ai_num].prev_y = game->npcs[ai_num].position.y;

        game->npcs[ai_num].current_marker = 0;
      }
    } else {
      if (start == true) {
        float value;
        // Match Python preprocessing exactly:
        // speed = tanh(speed / 50.0)
        // azimuth -> (sin, cos)
        float speed_norm = tanhf(game->npcs[ai_num].speed / 50.0f);
        float game_state[5] = {forward_err / norm, right_err / norm, speed_norm, sinf(heading), cosf(heading)};
        ac_forward(&(game->npcs[ai_num].model), game_state, game->npcs[ai_num].last_action, &value);
      }
    }
  }
  player_update(&(game->player), input_manager_get_controller_actions(input_manager), input_manager_get_controller_action_list_length(input_manager));

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

    water_update_uniforms(&(game->water), api_common, &(window->gbuffer->gbuffer_common), window->renderer.renderer_settings.width, window->renderer.renderer_settings.height);
    sprite_manager_update(&(game->sprite_manager), delta_time);
    sprite_manager_update_uniforms(&(game->sprite_manager), api_common, &(window->gbuffer->gbuffer_common));

    // GBuffer, only texture that needs to be multisampled
    gbuffer_start(window->gbuffer, &(window->swap_chain->swap_chain_common), window->renderer.renderer_settings.msaa_samples);

    water_render(&(game->water), &(window->gbuffer->gbuffer_common));

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
