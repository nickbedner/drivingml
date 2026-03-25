#define WIN32_LEAN_AND_MEAN
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <Windows.h>

////////

#include "drivingml/game.h"

static float wrap_angle_0_2pi(float a) {
  const float tau = 2.0f * (float)M_PI;

  while (a < 0.0f) a += tau;
  while (a >= tau) a -= tau;

  return a;
}

static float yaw_from_xy(float x, float y) {
  return atan2f(y, x);
}

static uint32_t car_frame_from_camera(float heading, float steer, vec3 car_pos, vec3d camera_pos) {
  const uint32_t BASE_FRAME_COUNT = 8;
  const uint32_t TURN_LEFT_FRAME = 8;
  const uint32_t TURN_RIGHT_FRAME = 9;
  const uint32_t FRONT_FRAME = 4;

  const float TURN_DEADZONE = 0.25f;

  // Direction from car -> camera in world xy
  float to_cam_x = (float)(camera_pos.x - (double)car_pos.x);
  float to_cam_y = (float)(camera_pos.y - (double)car_pos.y);
  float camera_yaw = yaw_from_xy(to_cam_x, to_cam_y);

  // Car forward direction in world XY
  float car_forward_yaw = yaw_from_xy(-cosf(heading), -sinf(heading));

  // 0 means camera is looking at the FRONT of the car
  float relative_yaw = wrap_angle_0_2pi(camera_yaw - car_forward_yaw);

  float step = (2.0f * (float)M_PI) / (float)BASE_FRAME_COUNT;

  uint32_t frame = (uint32_t)((relative_yaw + 0.5f * step) / step);
  frame %= BASE_FRAME_COUNT;

  // Only use the extra steering frames when we're looking at the front
  if (frame == FRONT_FRAME) {
    if (steer < -TURN_DEADZONE)
      return TURN_LEFT_FRAME;
    if (steer > TURN_DEADZONE)
      return TURN_RIGHT_FRAME;
  }

  return frame;
}
static quat sprite_billboard_rotation(vec3 car_pos, vec3d camera_pos) {
  float to_cam_x = (float)(camera_pos.x - (double)car_pos.x);
  float to_cam_y = (float)(camera_pos.y - (double)car_pos.y);
  float camera_yaw = yaw_from_xy(to_cam_x, to_cam_y);

  mat4 rot = mat4_rotate(MAT4_IDENTITY, (float)-M_PI / 2.0f, (vec3){.x = 0.5f, .y = 0.0f, .z = 0.0f});

  // Use camera yaw instead of heading
  rot = mat4_rotate(rot, -camera_yaw - (float)M_PI / 2.0f, (vec3){.x = 0.0f, .y = 1.0f, .z = 0.0f});

  return mat4_to_quaternion(rot);
}

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
  marker->sprite_common.position = (vec3){.x = x, .y = y, .z = 2.35f * 2.5f};
  marker->sprite_common.scale = (vec3){.x = 2.5f, .y = 2.5f, .z = 0.0f};
  mat4 marker_rotation_0 = mat4_rotate(MAT4_IDENTITY, (float)-M_PI / 2, (vec3){.x = 0.5, .y = 0.0, .z = 0.0});
  marker_rotation_0 = mat4_rotate(marker_rotation_0, (float)M_PI, (vec3){.x = 0.0, .y = 1.0, .z = 0.0});
  marker->sprite_common.rotation = mat4_to_quaternion(marker_rotation_0);
}

static void load_map_from_xml(struct Game* game, struct Mana* mana, const char* xml_path, const char* map_name) {
  struct XmlNode* root = xml_parser_load_xml_file(xml_path);
  if (!root)
    return;

  struct XmlNode* map_node = xml_node_get_child_with_attribute(root, "map", "name", map_name);

  if (!map_node) {
    xml_parser_delete(root);
    return;
  }

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
    game->track = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), tex);

    float scale = sx ? (float)atof(sx) : 25.0f;
    float x = px ? (float)atof(px) : 0.0f;
    float y = py ? (float)atof(py) : 0.0f;

    game->track->sprite_common.position = (vec3){.x = x, .y = y, .z = 0};
    game->track->sprite_common.scale = (vec3){.x = scale, .y = scale, .z = 0};

    mat4 rot = mat4_rotate(MAT4_IDENTITY, (float)M_PI, (vec3){.x = 0.0, .y = 0.5, .z = 0.0});
    game->track->sprite_common.rotation = mat4_to_quaternion(rot);
  }

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

        if (!x_str || !y_str)
          continue;

        float x = (float)atof(x_str);
        float y = (float)atof(y_str);

        game->marker[i] = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), "/textures/marker.png");

        place_marker(game->marker[i], x, y);
      }
    }
  }

  struct XmlNode* obstacle_node = xml_node_get_child(map_node, "obstacle");

  if (obstacle_node) {
    struct ArrayList* tree_list = xml_node_get_children(obstacle_node, "tree");

    if (tree_list) {
      size_t count = array_list_size(tree_list);
      game->total_trees = (int)count;

      for (size_t i = 0; i < count && i < MAX_TREES; i++) {
        struct XmlNode* tree = (struct XmlNode*)array_list_get(tree_list, i);

        char* x_str = xml_node_get_attribute(tree, "x");
        char* y_str = xml_node_get_attribute(tree, "y");

        if (!x_str || !y_str)
          continue;

        float x = (float)atof(x_str);
        float y = (float)atof(y_str);

        game->trees[i] = sprite_manager_add_sprite(
            &(game->sprite_manager),
            &(mana->api.api_common),
            "/textures/tree.png");

        game->trees[i]->sprite_common.position = (vec3){.x = x, .y = y, .z = 4.5f};
        game->trees[i]->sprite_common.scale = (vec3){.x = 5.0f, .y = 5.0f, .z = 0.0f};

        mat4 rot = mat4_rotate(MAT4_IDENTITY, -(float)M_PI / 2, (vec3){.x = 0.5, .y = 0, .z = 0});
        game->trees[i]->sprite_common.rotation = mat4_to_quaternion(rot);
      }
    }
  }

  xml_parser_delete(root);
}

void game_init(struct Game* game, struct Mana* mana, struct Window* window) {
  game->window = window;

  player_init(&(game->player), 1, game->window->renderer.renderer_settings.height);

  game->previous_reward = 0.0f;

  // TODO: Engine should decide max anisotropy based on device capabilities, not hardcoded here, and also loaded from settings. So that whol part will likely be removed from struct?
  struct TextureSettings sprite_texture_settings = (struct TextureSettings){.filter_type = FILTER_NEAREST, .mode_type = MODE_CLAMP_TO_EDGE, .format_type = FORMAT_R8G8B8A8_UNORM, .mip_type = MIP_GENERATE, .mip_count = 5, .premultiplied_alpha = true, .max_anisotropy = 1.0f};
  texture_manager_init(&(game->texture_manager), &(mana->api.api_common));
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/spritesheet.png", true);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/water.png", true);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/rb.png", true);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/whispy.png", true);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/fence.png", true);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/marker.png", true);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/floor_plane.png", true);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/barrel1.png", true);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/barrel2.png", true);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/tree.png", true);
  sprite_texture_settings = (struct TextureSettings){.filter_type = FILTER_ANISOTROPIC, .mode_type = MODE_CLAMP_TO_EDGE, .format_type = FORMAT_R8G8B8A8_UNORM, .mip_type = MIP_GENERATE, .mip_count = 5, .premultiplied_alpha = true, .max_anisotropy = 16.0f};
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/track.png", true);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/circuit.png", true);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/startfinish.png", true);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/cloud.png", true);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/map.png", true);
  sprite_texture_settings = (struct TextureSettings){.filter_type = FILTER_TRILINEAR, .mode_type = MODE_REPEAT, .format_type = FORMAT_R8G8B8A8_UNORM, .mip_type = MIP_CUSTOM, .mip_count = 5, .premultiplied_alpha = true, .max_anisotropy = 1.0f};
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/waterm1.png", false);
  sprite_texture_settings = (struct TextureSettings){.filter_type = FILTER_NEAREST, .mode_type = MODE_CLAMP_TO_EDGE, .format_type = FORMAT_R8G8B8A8_UNORM, .mip_type = MIP_NONE, .mip_count = 1, .premultiplied_alpha = true, .max_anisotropy = 1.0f};
  const char* grey_kart_frames[] = {
      "/textures/aikartgrey/tile000.png",
      "/textures/aikartgrey/tile001.png",
      "/textures/aikartgrey/tile002.png",
      "/textures/aikartgrey/tile003.png",
      "/textures/aikartgrey/tile004.png",
      "/textures/aikartgrey/tile005.png",
      "/textures/aikartgrey/tile006.png",
      "/textures/aikartgrey/tile007.png",
      "/textures/aikartgrey/tile008.png",
      "/textures/aikartgrey/tile009.png"};
  texture_manager_add_array(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/aikartgrey", grey_kart_frames, 10);
  const char* red_kart_frames[] = {
      "/textures/aikartred/tile000.png",
      "/textures/aikartred/tile001.png",
      "/textures/aikartred/tile002.png",
      "/textures/aikartred/tile003.png",
      "/textures/aikartred/tile004.png",
      "/textures/aikartred/tile005.png",
      "/textures/aikartred/tile006.png",
      "/textures/aikartred/tile007.png",
      "/textures/aikartred/tile008.png",
      "/textures/aikartred/tile009.png"};
  texture_manager_add_array(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/aikartred", red_kart_frames, 10);
  const char* cyan_kart_frames[] = {
      "/textures/aikartcyan/tile000.png",
      "/textures/aikartcyan/tile001.png",
      "/textures/aikartcyan/tile002.png",
      "/textures/aikartcyan/tile003.png",
      "/textures/aikartcyan/tile004.png",
      "/textures/aikartcyan/tile005.png",
      "/textures/aikartcyan/tile006.png",
      "/textures/aikartcyan/tile007.png",
      "/textures/aikartcyan/tile008.png",
      "/textures/aikartcyan/tile009.png"};
  texture_manager_add_array(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/aikartcyan", cyan_kart_frames, 10);
  const char* green_kart_frames[] = {
      "/textures/aikartgreen/tile000.png",
      "/textures/aikartgreen/tile001.png",
      "/textures/aikartgreen/tile002.png",
      "/textures/aikartgreen/tile003.png",
      "/textures/aikartgreen/tile004.png",
      "/textures/aikartgreen/tile005.png",
      "/textures/aikartgreen/tile006.png",
      "/textures/aikartgreen/tile007.png",
      "/textures/aikartgreen/tile008.png",
      "/textures/aikartgreen/tile009.png"};
  texture_manager_add_array(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/aikartgreen", green_kart_frames, 10);
  const char* purple_kart_frames[] = {
      "/textures/aikartpurple/tile000.png",
      "/textures/aikartpurple/tile001.png",
      "/textures/aikartpurple/tile002.png",
      "/textures/aikartpurple/tile003.png",
      "/textures/aikartpurple/tile004.png",
      "/textures/aikartpurple/tile005.png",
      "/textures/aikartpurple/tile006.png",
      "/textures/aikartpurple/tile007.png",
      "/textures/aikartpurple/tile008.png",
      "/textures/aikartpurple/tile009.png"};
  texture_manager_add_array(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/aikartpurple", purple_kart_frames, 10);

  sprite_manager_init(&(game->sprite_manager), &(game->texture_manager), &(mana->api.api_common), window->renderer.renderer_settings.width, window->renderer.renderer_settings.height, window->swap_chain->swap_chain_common.supersample_scale, &(window->gbuffer->gbuffer_common), window->renderer.renderer_settings.msaa_samples, 128);

  char path[MAX_LENGTH_OF_PATH] = {0};
  snprintf(path, MAX_LENGTH_OF_PATH, "%s/maps.xml", mana->api.api_common.asset_directory);
  load_map_from_xml(game, mana, path, "track0");

  // game->fence = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), "/textures/fence.png");
  // game->fence->sprite_common.position = (vec3){.x = 0, .y = 0.0f, .z = 2.5f};
  // game->fence->sprite_common.scale = (vec3){.x = 5.0f, .y = 5.0f, .z = 0.0f};
  // mat4 fence_rotation = mat4_rotate(MAT4_IDENTITY, -M_PI / 2, (vec3){.x = 0.5, .y = 0.0, .z = 0.0});
  // fence_rotation = mat4_rotate(fence_rotation, M_PI, (vec3){.x = 0.0, .y = 1.0, .z = 0.0});
  // game->fence->sprite_common.rotation = mat4_to_quaternion(fence_rotation);

  // game->cloud = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), "/textures/cloud.png");
  // game->cloud->sprite_common.position = (vec3){.x = 0, .y = -5000.0f, .z = 2.5f};
  // game->cloud->sprite_common.scale = (vec3){.x = 500.0f, .y = 500.0f, .z = 0.0f};
  // mat4 cloud_rotation = mat4_rotate(MAT4_IDENTITY, -M_PI / 2, (vec3){.x = 0.5, .y = 0.0, .z = 0.0});
  // cloud_rotation = mat4_rotate(cloud_rotation, M_PI, (vec3){.x = 0.0, .y = 1.0, .z = 0.0});
  // game->cloud->sprite_common.rotation = mat4_to_quaternion(cloud_rotation);

  // TODO: This needs to be 3D, sprite sorting messing it all up
  // game->floor_plane = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), L"/textures/floor_plane.png");
  // game->floor_plane->sprite_common.position = (vec3){.x = 0.0f, .y = 0.0f, .z = -5.1f};
  // game->floor_plane->sprite_common.scale = (vec3){.x = 50000.0f, .y = 50000.0f, .z = 0.0f};
  // mat4 rot = mat4_rotate(MAT4_IDENTITY, M_PI, (vec3){0.0, 0.5, 0.0});
  // game->floor_plane->sprite_common.rotation = mat4_to_quaternion(rot);
  //
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

  game->starting_pos = (vec3){.x = 175.0f, .y = 20.0f, .z = 0.75f};
  game->starting_heading = 0.0f;

  if (EVAL_MODE)
    game->current_npcs += MAX_NPCS;
  else
    game->current_npcs += 1;

  for (int32_t npc_num = 0; npc_num < game->current_npcs; npc_num++) {
    if (npc_num == 0) {
      if (EVAL_MODE == false)
        load_ac_model("checkpoints/ac_weights.bin", &(game->npcs[npc_num].model));
      game->npcs[npc_num].sprite = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), "/textures/aikartgrey");
    } else if (npc_num == 1) {
      game->npcs[npc_num].sprite = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), "/textures/aikartred");
      load_ac_model("checkpoints/ac_weights100.bin", &(game->npcs[npc_num].model));
    } else if (npc_num == 2) {
      game->npcs[npc_num].sprite = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), "/textures/aikartcyan");
      load_ac_model("checkpoints/ac_weights300.bin", &(game->npcs[npc_num].model));
    } else if (npc_num == 3) {
      game->npcs[npc_num].sprite = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), "/textures/aikartgreen");
      load_ac_model("checkpoints/ac_weights500.bin", &(game->npcs[npc_num].model));
    }
    game->npcs[npc_num].speed = 0.0f;
    game->npcs[npc_num].position = (vec3){.x = game->starting_pos.x - ((float)npc_num * 8), .y = game->starting_pos.y + ((float)(npc_num % 4) * 5.0f), .z = game->starting_pos.z};
    game->npcs[npc_num].sprite->sprite_common.position = game->npcs[npc_num].position;
    game->npcs[npc_num].sprite->sprite_common.scale = (vec3){.x = 5.0f, .y = 5.0f, .z = 0.0f};
    game->npcs[npc_num].heading = game->starting_heading;  // M_PI / 2.0f;  // facing down -Y
    game->npcs[npc_num].current_marker = 0;
    game->npcs[npc_num].last_action[0] = 0.0f;
    game->npcs[npc_num].last_action[1] = 0.0f;
    game->npcs[npc_num].prev_y = game->npcs[npc_num].position.y;
  }

  game->camera_current_follow_kart = 0;

  game->flag1 = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), "/textures/marker.png");
  place_marker(game->flag1, 165.0f, 0.0f);
  game->flag2 = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), "/textures/marker.png");
  place_marker(game->flag2, -165.0f, 0.0f);

  water_shader_init(&(game->water_shader), &(mana->api.api_common), window->renderer.renderer_settings.width, window->renderer.renderer_settings.height, window->swap_chain->swap_chain_common.supersample_scale, &(window->gbuffer->gbuffer_common), window->renderer.renderer_settings.msaa_samples, 3);
  water_init(&(game->water), &(mana->api.api_common), &(game->water_shader.shader), texture_manager_get(game->sprite_manager.sprite_manager_common.texture_manager, "/textures/waterm1.png"));
  game->water.water_common.position = (vec3){.x = 0.0f, .y = 0.0f, .z = -5.0f};
  game->water.water_common.scale = (vec3){.x = 1024.0f, .y = 1024.0f, .z = 1.0f};
}

void game_delete(struct Game* game, struct Mana* mana) {
  water_delete(&(game->water), &(mana->api.api_common));

  closesocket(game->sock);
  WSACleanup();

  struct APICommon* api_common = &(mana->api.api_common);

  // TODO: Throw this in a prepare delete function or something
  swap_chain_prepare_delete(game->window->swap_chain, api_common);

  sprite_manager_delete(&(game->sprite_manager), api_common);

  texture_manager_delete(&(game->texture_manager), api_common);
}

void game_update(struct Game* game, struct Mana* mana, double delta_time) {
  struct Window* window = game->window;
  struct InputManager* input_manager = &window->input_manager;

  bool done = false;

  // Delta time clamping, good for ML but not so good for game?
  if (delta_time > 0.05)
    delta_time = 0.05;

  // Hardcoded to episode length of 45 seconds before timeout
  // If I can complete course in about 30 secnds then AI should have 1.3x to 1.8x buffer
  game->timer++;
  if (game->timer > 2700 && !EVAL_MODE) {
    printf("Episode timed out\n");
    game->timer = 0;
    done = true;
  }

  game->start_timer++;
  bool start = false;
  if (game->start_timer > 180)
    start = true;

  const float speed_scale = 144.0f / 60.0f;  // 2.4
  float move_speed = 30.0f * speed_scale;
  float rotation_speed = 1.5f * speed_scale;
  vec3d cam_pos = camera_get_pos(&game->player.camera);

  for (int t = 0; t < game->total_trees; t++) {
    game->trees[t]->sprite_common.rotation =
        sprite_billboard_rotation(game->trees[t]->sprite_common.position, cam_pos);
  }

  static bool prev_left_pressed = false;
  static bool prev_right_pressed = false;

  bool left_pressed = false;
  bool right_pressed = false;

  bool w_pressed = false;
  bool a_pressed = false;
  bool s_pressed = false;
  bool d_pressed = false;

  for (size_t controller_num = 0; controller_num < input_manager->controllers.size; controller_num++) {
    struct Controller* controller = (struct Controller*)array_list_get(&(input_manager->controllers), controller_num);
    if (controller->controller_common.controller_type == CONTROLLER_KEYBOARD_MOUSE) {
      struct KeyboardMouseController* keyboard_mouse_controller =
          &(controller->controller_common.keyboard_mouse_controller);

      if (keyboard_mouse_controller->keys[KEY_LEFT].state == PRESSED)
        left_pressed = true;
      if (keyboard_mouse_controller->keys[KEY_RIGHT].state == PRESSED)
        right_pressed = true;

      if (keyboard_mouse_controller->keys[KEY_W].state == PRESSED)
        w_pressed = true;
      if (keyboard_mouse_controller->keys[KEY_S].state == PRESSED)
        s_pressed = true;
      if (keyboard_mouse_controller->keys[KEY_A].state == PRESSED)
        a_pressed = true;
      if (keyboard_mouse_controller->keys[KEY_D].state == PRESSED)
        d_pressed = true;
    }
  }

  if (game->current_npcs > 0) {
    if (left_pressed && !prev_left_pressed) {
      game->camera_current_follow_kart--;
      if (game->camera_current_follow_kart < 0)
        game->camera_current_follow_kart = game->current_npcs - 1;
    }

    if (right_pressed && !prev_right_pressed) {
      game->camera_current_follow_kart++;
      if (game->camera_current_follow_kart >= game->current_npcs)
        game->camera_current_follow_kart = 0;
    }
  }

  prev_left_pressed = left_pressed;
  prev_right_pressed = right_pressed;

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

    if (ai_num == 0 && EVAL_MODE == true) {
      steer = 0.0f;
      throttle = 0.0f;

      if (w_pressed)
        throttle = 1.0f;
      if (s_pressed)
        throttle = -1.0f;
      if (a_pressed)
        steer = -1.0f;
      if (d_pressed)
        steer = 1.0f;
    }

    float angle = -rotation_speed * (float)delta_time * steer;

    // Update car heading
    game->npcs[ai_num].heading += angle;

    game->npcs[ai_num].speed += throttle * move_speed * (float)delta_time;

    // Clamp speed
    if (game->npcs[ai_num].speed > 50.0f * speed_scale)
      game->npcs[ai_num].speed = 50.0f * speed_scale;
    if (game->npcs[ai_num].speed < -20.0f * speed_scale)
      game->npcs[ai_num].speed = -20.0f * speed_scale;

    // Movement + progress based reward
    float heading = game->npcs[ai_num].heading;
    vec3 forward_vel = (vec3){.x = -(float)cosf(heading), .y = -(float)sinf(heading), .z = 0.0f};

    // Current marker position
    vec3 marker_pos = game->marker[game->npcs[ai_num].current_marker]->sprite_common.position;

    // Distance BEFORE movement
    float dx_before = marker_pos.x - game->npcs[ai_num].position.x;
    float dy_before = marker_pos.y - game->npcs[ai_num].position.y;
    float dist_before = sqrtf(dx_before * dx_before + dy_before * dy_before);

    bool hit_tree = false;

    ///////////////////////////////////////////////////
    // Start of reward calculation
    ///////////////////////////////////////////////////
    float reward = 0.0f;
    float speed_before_move = game->npcs[ai_num].speed;
    vec3 prev_pos = game->npcs[ai_num].position;

    //  Move car
    game->npcs[ai_num].position.x += forward_vel.x * game->npcs[ai_num].speed * (float)delta_time;
    game->npcs[ai_num].position.y += forward_vel.y * game->npcs[ai_num].speed * (float)delta_time;

    const float TREE_RADIUS = 1.75f;
    const float TREE_SKIN = 0.20f;  // extra push-out margin
    const float BOUNCE_RESTITUTION = 1.5f;
    const float MIN_BOUNCE_SPEED = 15.0f;
    const float TREE_AVOID_TURN = 0.35f;  // about 20 degrees
    const float TREE_SIDE_EPS = 0.05f;

    for (int t = 0; t < game->total_trees; t++) {
      vec3 tree_pos = game->trees[t]->sprite_common.position;

      float dx = game->npcs[ai_num].position.x - tree_pos.x;
      float dy = game->npcs[ai_num].position.y - tree_pos.y;
      float dist = sqrtf(dx * dx + dy * dy);

      if (dist < TREE_RADIUS) {
        hit_tree = true;

        // Start from pre-move position so we do not stay embedded in the tree
        float resolve_x = prev_pos.x;
        float resolve_y = prev_pos.y;

        float rdx = resolve_x - tree_pos.x;
        float rdy = resolve_y - tree_pos.y;
        float rdist = sqrtf(rdx * rdx + rdy * rdy);

        float nx, ny;
        if (rdist > 1e-4f) {
          nx = rdx / rdist;
          ny = rdy / rdist;
        } else if (dist > 1e-4f) {
          nx = dx / dist;
          ny = dy / dist;
        } else {
          // Fallback: use opposite of forward direction
          nx = -forward_vel.x;
          ny = -forward_vel.y;
        }

        // Push slightly outside the tree, not exactly on the boundary
        game->npcs[ai_num].position.x = tree_pos.x + nx * (TREE_RADIUS + TREE_SKIN);
        game->npcs[ai_num].position.y = tree_pos.y + ny * (TREE_RADIUS + TREE_SKIN);

        // Estimate impact speed along the collision normal
        float vx = forward_vel.x * speed_before_move;
        float vy = forward_vel.y * speed_before_move;
        float impact_speed = fabsf(vx * nx + vy * ny);

        // Bounce backward
        game->npcs[ai_num].speed = -fmaxf(MIN_BOUNCE_SPEED, impact_speed * BOUNCE_RESTITUTION);

        // Turn slightly away from the tree so the AI does not keep rehitting it
        float current_heading = game->npcs[ai_num].heading;

        float fx = -cosf(current_heading);
        float fy = -sinf(current_heading);
        float rx = fy;
        float ry = -fx;

        // Tree position in carspace
        float to_tree_x = tree_pos.x - game->npcs[ai_num].position.x;
        float to_tree_y = tree_pos.y - game->npcs[ai_num].position.y;
        float tree_side = to_tree_x * rx + to_tree_y * ry;

        if (fabsf(tree_side) < TREE_SIDE_EPS) {
          float to_marker_x = marker_pos.x - game->npcs[ai_num].position.x;
          float to_marker_y = marker_pos.y - game->npcs[ai_num].position.y;
          tree_side = to_marker_x * rx + to_marker_y * ry;
        }

        if (tree_side > 0.0f)
          game->npcs[ai_num].heading += TREE_AVOID_TURN;
        else
          game->npcs[ai_num].heading -= TREE_AVOID_TURN;

        game->npcs[ai_num].heading = wrap_angle_0_2pi(game->npcs[ai_num].heading);

        reward -= 4.0f;
        break;
      }
    }

    heading = game->npcs[ai_num].heading;

    // Apply damping
    float damping = 2.0f;
    game->npcs[ai_num].speed *= expf((float)((double)-damping * delta_time));

    // Update sprite + camera
    game->npcs[ai_num].sprite->sprite_common.position = game->npcs[ai_num].position;
    game->npcs[ai_num].sprite->sprite_common.rotation = sprite_billboard_rotation(game->npcs[ai_num].position, cam_pos);
    game->npcs[ai_num].sprite->sprite_common.frame_layer = car_frame_from_camera(game->npcs[ai_num].heading, steer, game->npcs[ai_num].position, cam_pos);

    //  Distance AFTER movement
    float dx_after = marker_pos.x - game->npcs[ai_num].position.x;
    float dy_after = marker_pos.y - game->npcs[ai_num].position.y;
    float dist_after = sqrtf(dx_after * dx_after + dy_after * dy_after);

    // Main dense signal: reward distance reduction
    float progress = dist_before - dist_after;
    if (!hit_tree)
      reward += 0.1f * progress;

    const float checkpoint_radius = 15.0f;
    if (!hit_tree && dist_after < checkpoint_radius) {
      reward += 2.0f;  // checkpoint bonus

      game->npcs[ai_num].current_marker++;

      if (game->npcs[ai_num].current_marker >= game->total_markers) {
        game->npcs[ai_num].current_marker = 0;
        reward += 5.0f;  // lap bonus
        // done = true;
        // game->timer = 0;
      }
    }

    // Penalize reversing
    if (game->npcs[ai_num].speed < 0.0f)
      reward -= 0.05f;
    // Small time penalty
    reward -= 0.005f;
    // Steering penalty
    reward -= 0.01f * steer * steer;
    reward -= 0.02f * fabsf(angle);

    // Use the current target marker after checkpoint update so we head towards the next one if we just passed the checkpoint
    vec3 next_marker = game->marker[game->npcs[ai_num].current_marker]->sprite_common.position;

    // World delta to target, basically aiming towards it
    float dxw = next_marker.x - game->npcs[ai_num].position.x;
    float dyw = next_marker.y - game->npcs[ai_num].position.y;
    // Car forward/right basis
    float fx = -cosf(game->npcs[ai_num].heading);
    float fy = -sinf(game->npcs[ai_num].heading);
    float rx = fy;
    float ry = -fx;
    // Marker error in car frame
    float forward_err = dxw * fx + dyw * fy;
    float right_err = dxw * rx + dyw * ry;

    // Normalization constants
    const float norm = 150.0f;
    const float obstacle_norm = 100.0f;

    // Pick the most relevant tree ahead of the car
    bool found_tree_ahead = false;
    float best_forward = obstacle_norm;
    float best_right = 0.0f;
    float best_score = FLT_MAX;

    for (int t = 0; t < game->total_trees; t++) {
      vec3 tree_pos = game->trees[t]->sprite_common.position;

      float dx = tree_pos.x - game->npcs[ai_num].position.x;
      float dy = tree_pos.y - game->npcs[ai_num].position.y;

      float forward = dx * fx + dy * fy;
      float right = dx * rx + dy * ry;

      if (forward <= 0.0f)
        continue;

      float score = forward + 2.0f * fabsf(right);
      if (score < best_score) {
        found_tree_ahead = true;
        best_score = score;
        best_forward = forward;
        best_right = right;
      }
    }

    float tree_dx = 1.0f;
    float tree_dy = 0.0f;
    if (found_tree_ahead) {
      tree_dx = best_forward / obstacle_norm;
      tree_dy = best_right / obstacle_norm;
    }

    // Clamp to sane range
    if (tree_dx > 1.0f)
      tree_dx = 1.0f;
    if (tree_dx < -1.0f)
      tree_dx = -1.0f;
    if (tree_dy > 1.0f)
      tree_dy = 1.0f;
    if (tree_dy < -1.0f)
      tree_dy = -1.0f;

    if (found_tree_ahead && tree_dx < 0.25f) {
      float lateral = fabsf(tree_dy);
      if (lateral < 0.15f) {
        float forward_term = (0.25f - tree_dx) / 0.25f;
        float lateral_term = (0.15f - lateral) / 0.15f;
        reward -= 0.15f * forward_term * lateral_term;
      }
    }

    if (!EVAL_MODE) {
      struct Packet {
        float dx;       // Next checkpoint x relative to car direction
        float dy;       // Next checkpoint y relative to car direction
        float speed;    // Car speed
        float azimuth;  // Car angle
        float tree_dx;  // Next forward facing tree x relative to car direction
        float tree_dy;  // Next forward facing tree y relative to car direction
        float reward;   // Reward for this step
        int done;       // Whether the episode is done
      };

      struct Packet packet;
      // Normalize to roughly [-1,1]
      packet.dx = forward_err / norm;
      packet.dy = right_err / norm;
      packet.speed = game->npcs[ai_num].speed;
      packet.azimuth = heading;
      packet.tree_dx = tree_dx;
      packet.tree_dy = tree_dy;
      packet.reward = reward;
      packet.done = done;

      send(game->sock, (char*)&packet, sizeof(packet), 0);

      if (recv_all(game->sock, (char*)game->npcs[ai_num].last_action, sizeof(game->npcs[ai_num].last_action)) <= 0) {
        game->npcs[ai_num].last_action[0] = 0.0f;
        game->npcs[ai_num].last_action[1] = 0.0f;
      }

      if (done) {
        // game->timer = 0;

        game->npcs[ai_num].speed = 0.0f;
        game->npcs[ai_num].position = game->starting_pos;
        game->npcs[ai_num].sprite->sprite_common.position = game->npcs[ai_num].position;
        game->npcs[ai_num].sprite->sprite_common.scale = (vec3){.x = 5.0f, .y = 5.0f, .z = 0.0f};
        game->npcs[ai_num].heading = game->starting_heading;
        game->npcs[ai_num].sprite->sprite_common.rotation = sprite_billboard_rotation(game->npcs[ai_num].position, cam_pos);
        game->npcs[ai_num].sprite->sprite_common.frame_layer = car_frame_from_camera(game->npcs[ai_num].heading, steer, game->npcs[ai_num].position, cam_pos);

        game->npcs[ai_num].last_action[0] = 0.0f;
        game->npcs[ai_num].last_action[1] = 0.0f;
        game->npcs[ai_num].prev_y = game->npcs[ai_num].position.y;

        game->npcs[ai_num].current_marker = 0;
      }
    } else {
      if (start == true) {
        float value;
        float speed_norm = tanhf(game->npcs[ai_num].speed / 120.0f);
        float game_state[7] = {forward_err / norm, right_err / norm, speed_norm, sinf(heading), cosf(heading), tree_dx, tree_dy};
        ac_forward(&(game->npcs[ai_num].model), game_state, game->npcs[ai_num].last_action, &value);
      }
    }
  }

  if (game->current_npcs > 0) {
    int follow = game->camera_current_follow_kart;

    if (follow < 0)
      follow = 0;
    if (follow >= game->current_npcs)
      follow = game->current_npcs - 1;

    game->player.look_at_pos = (vec3d){.x = (double)game->npcs[follow].position.x, .y = (double)game->npcs[follow].position.y, .z = (double)game->npcs[follow].position.z};
    game->player.camera.look_at_azimuth = (double)game->npcs[follow].heading + M_PI;
  }

  player_update(&(game->player), input_manager_get_controller_actions(input_manager), input_manager_get_controller_action_list_length(input_manager));

  // Make this always facing toward the camera
  for (int marker_num = 0; marker_num < game->total_markers; marker_num++) {
    mat4 marker_rotation = mat4_rotate(MAT4_IDENTITY, -(float)M_PI / 2.0f, (vec3){.x = 0.5f, .y = 0.0f, .z = 0.0f});
    marker_rotation = mat4_rotate(marker_rotation, (float)M_PI / 2.0f, (vec3){.x = 0.0f, .y = 1.0f, .z = 0.0f});
    game->marker[marker_num]->sprite_common.rotation = mat4_to_quaternion(mat4_rotate(marker_rotation, (float)-game->player.camera.look_at_azimuth, (vec3){.x = 0.0f, .y = 1.0f, .z = 0.0f}));
    // TODO: Commented out temporarily because we just want to hide markers for now
    // if (EVAL_MODE)
    game->marker[marker_num]->sprite_common.position.z = -10000.0f;
  }
  mat4 flag1_rotation = mat4_rotate(MAT4_IDENTITY, -(float)M_PI / 2.0f, (vec3){.x = 0.5f, .y = 0.0f, .z = 0.0f});
  flag1_rotation = mat4_rotate(flag1_rotation, (float)M_PI / 2.0f, (vec3){.x = 0.0f, .y = 1.0f, .z = 0.0f});
  game->flag1->sprite_common.rotation = mat4_to_quaternion(mat4_rotate(flag1_rotation, (float)-game->player.camera.look_at_azimuth, (vec3){.x = 0.0f, .y = 1.0f, .z = 0.0f}));
  mat4 flag2_rotation = mat4_rotate(MAT4_IDENTITY, -(float)M_PI / 2.0f, (vec3){.x = 0.5f, .y = 0.0f, .z = 0.0f});
  flag2_rotation = mat4_rotate(flag2_rotation, (float)M_PI / 2.0f, (vec3){.x = 0.0f, .y = 1.0f, .z = 0.0f});
  game->flag2->sprite_common.rotation = mat4_to_quaternion(mat4_rotate(flag2_rotation, (float)-game->player.camera.look_at_azimuth, (vec3){.x = 0.0f, .y = 1.0f, .z = 0.0f}));
}

void game_render(struct Game* game, struct Mana* mana, double delta_time) {
  struct APICommon* api_common = &(mana->api.api_common);
  struct Window* window = game->window;

  window->gbuffer->gbuffer_common.projection_matrix = camera_get_projection_matrix(&(game->player.camera), window);
  window->gbuffer->gbuffer_common.view_matrix = camera_get_view_matrix(&(game->player.camera));

  // Note: Wait for window to say it's ready to be resized
  if (!IsIconic(window->surface.hwnd)) {
    if (window->should_resize) {
      renderer_wait_for_device(&(window->renderer), window->api_common);

      shader_resize(&(game->water_shader.shader), api_common, window->renderer.renderer_settings.width, window->renderer.renderer_settings.height, window->swap_chain->swap_chain_common.supersample_scale);

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
