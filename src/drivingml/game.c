#define WIN32_LEAN_AND_MEAN
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <Windows.h>

////////

#include "drivingml/game.h"

internal r32 wrap_angle_0_2pi(r32 a) {
  const r32 tau = 2.0f * (r32)R32_PI;

  while (a < 0.0f) a += tau;
  while (a >= tau) a -= tau;

  return a;
}

internal r32 yaw_from_xz(r32 x, r32 z) {
  return real32_atan2(z, x);
}

internal u32 car_frame_from_camera(r32 heading, r32 steer, vec3 car_pos, vec3d camera_pos) {
  const u32 BASE_FRAME_COUNT = 8;
  const u32 TURN_LEFT_FRAME = 8;
  const u32 TURN_RIGHT_FRAME = 9;
  const u32 FRONT_FRAME = 4;

  const r32 TURN_DEADZONE = 0.25f;

  // Direction from car -> camera in world XZ
  r32 to_cam_x = (r32)(camera_pos.x - (r64)car_pos.x);
  r32 to_cam_z = (r32)(camera_pos.z - (r64)car_pos.z);
  r32 camera_yaw = yaw_from_xz(to_cam_x, to_cam_z);

  // Car forward direction in world XZ
  r32 car_back_yaw = yaw_from_xz(real32_cos(heading), -real32_sin(heading));
  r32 relative_yaw = wrap_angle_0_2pi(car_back_yaw - camera_yaw);
  r32 step = (2.0f * (r32)R32_PI) / (r32)BASE_FRAME_COUNT;

  u32 frame = (u32)((relative_yaw + 0.5f * step) / step);
  frame %= BASE_FRAME_COUNT;

  if (frame == FRONT_FRAME) {
    if (steer < -TURN_DEADZONE)
      return TURN_LEFT_FRAME;
    if (steer > TURN_DEADZONE)
      return TURN_RIGHT_FRAME;
  }

  return frame;
}

internal quat sprite_billboard_rotation(vec3 car_pos, vec3d camera_pos) {
  r32 to_cam_x = (r32)(camera_pos.x - (r64)car_pos.x);
  r32 to_cam_z = (r32)(camera_pos.z - (r64)car_pos.z);
  r32 yaw = real32_atan2(to_cam_x, to_cam_z);

  mat4 rot = mat4_rotate(MAT4_IDENTITY, yaw, (vec3){.x = 0.0f, .y = 1.0f, .z = 0.0f});
  return mat4_to_quaternion(rot);
}

internal i32 recv_all(SOCKET sock, char* buffer, i32 size) {
  i32 total = 0;
  i32 bytes;

  while (total < size) {
    bytes = recv(sock, buffer + total, size - total, 0);
    if (bytes <= 0) return bytes;
    total += bytes;
  }
  return total;
}

internal inline void place_marker(struct Sprite* marker, r32 x, r32 y) {
  marker->sprite_common.position = (vec3){.x = x, .y = 2.35f * 2.5f, .z = y};
  marker->sprite_common.scale = (vec3){.x = 2.5f, .y = 2.5f, .z = 0.0f};
  mat4 marker_rotation_0 = mat4_rotate(MAT4_IDENTITY, (r32)-R32_PI / 2, (vec3){.x = 0.5, .y = 0.0, .z = 0.0});
  marker_rotation_0 = mat4_rotate(marker_rotation_0, (r32)R32_PI, (vec3){.x = 0.0, .y = 1.0, .z = 0.0});
  marker->sprite_common.rotation = mat4_to_quaternion(marker_rotation_0);
}

internal void load_map_from_xml(struct Game* game, struct Mana* mana, const char* xml_path, const char* map_name) {
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
    game->track_model = model_cache_get(&(game->model_cache), &(mana->api.api_common), tex);

    r32 scale = sx ? (r32)atof(sx) : 25.0f;
    r32 x = px ? (r32)atof(px) : 0.0f;
    r32 y = py ? (r32)atof(py) : 0.0f;

    // game->track_model->model_common.position = (vec3){.x = x, .y = 0.0f, .z = y};
    game->track_model->model_common.scale = (vec3){.x = scale, .y = 0, .z = scale};

    // game->track = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), tex);
    //
    // r32 scale = sx ? (r32)atof(sx) : 25.0f;
    // r32 x = px ? (r32)atof(px) : 0.0f;
    // r32 y = py ? (r32)atof(py) : 0.0f;
    //
    // game->track->sprite_common.position = (vec3){.x = x, .y = 0.0f, .z = y};
    // game->track->sprite_common.scale = (vec3){.x = scale, .y = scale, .z = 0};
    //
    // mat4 rot = mat4_rotate(MAT4_IDENTITY, (r32)-R32_PI / 2.0f, (vec3){.x = 1.0f, .y = 0.0f, .z = 0.0f});
    // game->track->sprite_common.rotation = mat4_to_quaternion(rot);
  }

  struct XmlNode* markers_node = xml_node_get_child(map_node, "markers");

  if (markers_node) {
    struct ArrayList* marker_list = xml_node_get_children(markers_node, "marker");

    if (marker_list) {
      size_t count = array_list_size(marker_list);
      game->total_markers = (i32)count;

      for (size_t i = 0; i < count; i++) {
        struct XmlNode* marker = (struct XmlNode*)array_list_get(marker_list, i);

        char* x_str = xml_node_get_attribute(marker, "x");
        char* y_str = xml_node_get_attribute(marker, "y");

        if (!x_str || !y_str)
          continue;

        r32 x = (r32)atof(x_str);
        r32 y = (r32)atof(y_str);

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
      game->total_trees = (i32)count;

      for (size_t i = 0; i < count && i < MAX_TREES; i++) {
        struct XmlNode* tree = (struct XmlNode*)array_list_get(tree_list, i);

        char* x_str = xml_node_get_attribute(tree, "x");
        char* y_str = xml_node_get_attribute(tree, "y");

        if (!x_str || !y_str)
          continue;

        r32 x = (r32)atof(x_str);
        r32 y = (r32)atof(y_str);

        game->trees[i] = sprite_manager_add_sprite(
            &(game->sprite_manager),
            &(mana->api.api_common),
            "/textures/tree.png");

        game->trees[i]->sprite_common.position = (vec3){.x = x, .y = 4.5f, .z = y};
        game->trees[i]->sprite_common.scale = (vec3){.x = 5.0f, .y = 5.0f, .z = 0.0f};

        mat4 rot = mat4_rotate(MAT4_IDENTITY, -(r32)R32_PI / 2, (vec3){.x = 0.5, .y = 0, .z = 0});
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

  char fart_path[MAX_LENGTH_OF_PATH] = {0};
  snprintf(fart_path, MAX_LENGTH_OF_PATH, "%s/audio/fart.wav", mana->api.api_common.asset_directory);
  load_audio(fart_path, &(game->fart));
  // play_audio_wasapi(&(game->fart));

  // TODO: Engine should decide max anisotropy based on device capabilities, not hardcoded here, and also loaded from settings. So that whol part will likely be removed from struct?
  struct TextureSettings sprite_texture_settings = (struct TextureSettings){.filter_type = FILTER_NEAREST, .mode_type = MODE_CLAMP_TO_EDGE, .format_type = FORMAT_R8G8B8A8_UNORM, .mip_type = MIP_GENERATE, .mip_count = 5, .premultiplied_alpha = TRUE, .max_anisotropy = 1.0f};
  texture_manager_init(&(game->texture_manager), &(mana->api.api_common));
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/spritesheet.png", TRUE);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/water.png", TRUE);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/rb.png", TRUE);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/whispy.png", TRUE);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/fence.png", TRUE);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/marker.png", TRUE);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/floor_plane.png", TRUE);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/barrel1.png", TRUE);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/barrel2.png", TRUE);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/tree.png", TRUE);
  sprite_texture_settings = (struct TextureSettings){.filter_type = FILTER_ANISOTROPIC, .mode_type = MODE_CLAMP_TO_EDGE, .format_type = FORMAT_R8G8B8A8_UNORM, .mip_type = MIP_GENERATE, .mip_count = 5, .premultiplied_alpha = TRUE, .max_anisotropy = 16.0f};
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/track.png", TRUE);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/circuit.png", TRUE);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/startfinish.png", TRUE);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/cloud.png", TRUE);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/map.png", TRUE);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/models/testmodel/diffuse.png", TRUE);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/models/testmodel/albedo.png", TRUE);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/models/testmodel/normal.png", TRUE);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/models/testmodel/roughness.png", TRUE);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/models/testmodel/metallic.png", TRUE);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/models/testmodel/ao.png", TRUE);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/models/coin/coin.png", TRUE);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/models/coin/coinod.png", TRUE);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/models/coin/coinon.png", TRUE);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/models/coin/coinom.png", TRUE);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/models/ssc/Textures/coina.png", TRUE);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/models/ssc/Textures/coinn.png", TRUE);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/models/ssc/Textures/coinr.png", TRUE);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/models/ssc/Textures/coinm.png", TRUE);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/models/ssc/Textures/coinao.png", TRUE);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/models/Watermelon/watermelona.png", TRUE);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/models/Watermelon/watermelonn.png", TRUE);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/models/track/diffuse.png", TRUE);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/models/track/normal.png", TRUE);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/models/track/metallic.png", TRUE);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/models/track/roughness.png", TRUE);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/models/track/ao.png", TRUE);
  sprite_texture_settings = (struct TextureSettings){.filter_type = FILTER_TRILINEAR, .mode_type = MODE_REPEAT, .format_type = FORMAT_R8G8B8A8_UNORM, .mip_type = MIP_CUSTOM, .mip_count = 5, .premultiplied_alpha = TRUE, .max_anisotropy = 1.0f};
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/waterm1.png", FALSE);
  sprite_texture_settings = (struct TextureSettings){.filter_type = FILTER_NEAREST, .mode_type = MODE_CLAMP_TO_EDGE, .format_type = FORMAT_R8G8B8A8_UNORM, .mip_type = MIP_NONE, .mip_count = 1, .premultiplied_alpha = TRUE, .max_anisotropy = 1.0f};
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

  game->starting_pos = (vec3){.x = 175.0f, .y = 0.75f, .z = 20.0f};
  game->starting_heading = 0.0f;

  if (EVAL_MODE)
    game->current_npcs += MAX_NPCS;
  else
    game->current_npcs += 1;

  for (i32 npc_num = 0; npc_num < game->current_npcs; npc_num++) {
    if (npc_num == 0) {
      if (EVAL_MODE == FALSE)
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
    game->npcs[npc_num].position = (vec3){.x = game->starting_pos.x - ((r32)npc_num * 8), .y = game->starting_pos.y, .z = game->starting_pos.z + ((r32)(npc_num % 4) * 5.0f)};
    game->npcs[npc_num].sprite->sprite_common.position = game->npcs[npc_num].position;
    game->npcs[npc_num].sprite->sprite_common.scale = (vec3){.x = 5.0f, .y = 5.0f, .z = 0.0f};
    game->npcs[npc_num].heading = game->starting_heading;  // R32_PI / 2.0f;  // facing down -Y
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
  game->water.water_common.position = (vec3){.x = 0.0f, .y = -5.0f, .z = 0.0f};
  game->water.water_common.scale = (vec3){.x = 1024.0f, .y = 1.0f, .z = 1024.0f};

  model_cache_init(&(game->model_cache), &(mana->api.api_common), window->renderer.renderer_settings.width, window->renderer.renderer_settings.height, window->swap_chain->swap_chain_common.supersample_scale, &(window->gbuffer->gbuffer_common), window->renderer.renderer_settings.msaa_samples, 128);
  struct ModelSettings model_settings = (struct ModelSettings){
      .path = "./assets/models/testmodel/model.dae",
      .shader = &(game->model_cache.model_shader.shader),
      .diffuse_texture = texture_manager_get(&(game->texture_manager), "/models/testmodel/diffuse.png"),
      .normal_texture = texture_manager_get(&(game->texture_manager), "/models/testmodel/normal.png"),
      .metallic_texture = texture_manager_get(&(game->texture_manager), "/models/testmodel/metallic.png"),
      .roughness_texture = texture_manager_get(&(game->texture_manager), "/models/testmodel/roughness.png"),
      .ao_texture = texture_manager_get(&(game->texture_manager), "/models/testmodel/ao.png"),
      5};
  model_cache_add(&(game->model_cache), &(mana->api.api_common), &model_settings, 0, TRUE);
  game->test_model = model_cache_get(&(game->model_cache), &(mana->api.api_common), "./assets/models/testmodel/model.dae");

  struct ModelSettings model_static_settings = (struct ModelSettings){
      .path = "./assets/models/cube/cube.dae",
      .shader = &(game->model_cache.model_static_shader.shader),
      .diffuse_texture = texture_manager_get(&(game->texture_manager), "/models/testmodel/albedo.png"),
      .normal_texture = texture_manager_get(&(game->texture_manager), "/models/testmodel/normal.png"),
      .metallic_texture = texture_manager_get(&(game->texture_manager), "/models/testmodel/metallic.png"),
      .roughness_texture = texture_manager_get(&(game->texture_manager), "/models/testmodel/roughness.png"),
      .ao_texture = texture_manager_get(&(game->texture_manager), "/models/testmodel/ao.png"),
      5};
  model_cache_add(&(game->model_cache), &(mana->api.api_common), &model_static_settings, 1, FALSE);
  game->test_static_model = model_cache_get(&(game->model_cache), &(mana->api.api_common), "./assets/models/cube/cube.dae");
  game->test_static_model->model_common.scale = (vec3){.x = 1.0f, .y = 1.0f, .z = 1.0f};
  game->test_static_model->model_common.position = (vec3){.x = 5.0f, .y = 2.0f, .z = 0.0f};

  struct ModelSettings model_track_settings = (struct ModelSettings){
      .path = "./assets/models/track/track.dae",
      .shader = &(game->model_cache.model_static_shader.shader),
      .diffuse_texture = texture_manager_get(&(game->texture_manager), "/models/track/diffuse.png"),
      .normal_texture = texture_manager_get(&(game->texture_manager), "/models/track/normal.png"),
      .metallic_texture = texture_manager_get(&(game->texture_manager), "/models/track/metallic.png"),
      .roughness_texture = texture_manager_get(&(game->texture_manager), "/models/track/roughness.png"),
      .ao_texture = texture_manager_get(&(game->texture_manager), "/models/track/ao.png"),
      5};
  model_cache_add(&(game->model_cache), &(mana->api.api_common), &model_track_settings, 2, FALSE);

  // struct ModelSettings coin_settings = (struct ModelSettings){
  //     .path = "./assets/models/ssc/Coin.dae",
  //     .shader = &(game->model_cache.model_static_shader.shader),
  //     .diffuse_texture = texture_manager_get(&(game->texture_manager), "/models/ssc/Textures/coina.png"),
  //     .normal_texture = texture_manager_get(&(game->texture_manager), "/models/ssc/Textures/coinn.png"),
  //     .metallic_texture = texture_manager_get(&(game->texture_manager), "/models/ssc/Textures/coinm.png"),
  //     .roughness_texture = texture_manager_get(&(game->texture_manager), "/models/ssc/Textures/coinr.png"),
  //     .ao_texture = texture_manager_get(&(game->texture_manager), "/models/ssc/Textures/coinao.png"),
  //     5};
  // model_cache_add(&(game->model_cache), &(mana->api.api_common), &coin_settings, 1, FALSE);
  // game->coin_model = model_cache_get(&(game->model_cache), &(mana->api.api_common), "./assets/models/ssc/Coin.dae");
  struct ModelSettings coin_settings = (struct ModelSettings){
      .path = "./assets/models/Watermelon/watermelon.dae",
      .shader = &(game->model_cache.model_static_shader.shader),
      .diffuse_texture = texture_manager_get(&(game->texture_manager), "/models/Watermelon/watermelona.png"),
      .normal_texture = texture_manager_get(&(game->texture_manager), "/models/Watermelon/watermelonn.png"),
      .metallic_texture = texture_manager_get(&(game->texture_manager), "/models/ssc/Textures/coinm.png"),
      .roughness_texture = texture_manager_get(&(game->texture_manager), "/models/ssc/Textures/coinr.png"),
      .ao_texture = texture_manager_get(&(game->texture_manager), "/models/ssc/Textures/coinao.png"),
      5};
  model_cache_add(&(game->model_cache), &(mana->api.api_common), &coin_settings, 3, FALSE);
  game->coin_model = model_cache_get(&(game->model_cache), &(mana->api.api_common), "./assets/models/Watermelon/watermelon.dae");
  game->coin_model->model_common.scale = (vec3){.x = 1.5f, .y = 1.5f, .z = 1.5f};
  game->coin_model->model_common.position = (vec3){.x = 15.0f, .y = 4.0f, .z = 0.0f};

  char path[MAX_LENGTH_OF_PATH] = {0};
  snprintf(path, MAX_LENGTH_OF_PATH, "%s/maps.xml", mana->api.api_common.asset_directory);
  load_map_from_xml(game, mana, path, "track0");
}

void game_delete(struct Game* game, struct Mana* mana) {
  closesocket(game->sock);
  WSACleanup();

  struct APICommon* api_common = &(mana->api.api_common);
  // TODO: Throw this in a prepare delete function or something
  // Note: We need this before we start to teardown the rest of the game objects since they might be using the swap chain for rendering during their delete functions
  swap_chain_prepare_delete(game->window->swap_chain, api_common);
  water_delete(&(game->water), api_common);
  water_shader_delete(&(game->water_shader), api_common);
  sprite_manager_delete(&(game->sprite_manager), api_common);
  texture_manager_delete(&(game->texture_manager), api_common);
}

void game_update(struct Game* game, struct Mana* mana, r64 delta_time) {
  struct Window* window = game->window;
  struct InputManager* input_manager = &window->input_manager;

  b8 done = FALSE;

  // Delta time clamping, good for ML but not so good for game?
  if (delta_time > 0.05)
    delta_time = 0.05;

  // Hardcoded to episode length of 45 seconds before timeout
  // If I can complete course in about 30 secnds then AI should have 1.3x to 1.8x buffer
  game->timer++;
  if (game->timer > 2700 && !EVAL_MODE) {
    printf("Episode timed out\n");
    game->timer = 0;
    done = TRUE;
  }

  game->start_timer++;
  b8 start = FALSE;
  if (game->start_timer > 180)
    start = TRUE;

  const r32 speed_scale = 144.0f / 60.0f;  // 2.4
  r32 move_speed = 30.0f * speed_scale;
  r32 rotation_speed = 1.5f * speed_scale;
  vec3d cam_pos = camera_get_pos(&game->player.camera);

  for (i32 t = 0; t < game->total_trees; t++) {
    game->trees[t]->sprite_common.rotation =
        sprite_billboard_rotation(game->trees[t]->sprite_common.position, cam_pos);
  }

  persist b8 prev_left_pressed = FALSE;
  persist b8 prev_right_pressed = FALSE;

  b8 left_pressed = FALSE;
  b8 right_pressed = FALSE;

  b8 w_pressed = FALSE;
  b8 a_pressed = FALSE;
  b8 s_pressed = FALSE;
  b8 d_pressed = FALSE;

  for (size_t controller_num = 0; controller_num < input_manager->controllers.size; controller_num++) {
    struct Controller* controller = (struct Controller*)array_list_get(&(input_manager->controllers), controller_num);
    if (controller->controller_common.controller_type == CONTROLLER_KEYBOARD_MOUSE) {
      struct KeyboardMouseController* keyboard_mouse_controller =
          &(controller->controller_common.keyboard_mouse_controller);

      if (keyboard_mouse_controller->keys[KEY_LEFT].state == PRESSED)
        left_pressed = TRUE;
      if (keyboard_mouse_controller->keys[KEY_RIGHT].state == PRESSED)
        right_pressed = TRUE;

      if (keyboard_mouse_controller->keys[KEY_W].state == PRESSED)
        w_pressed = TRUE;
      if (keyboard_mouse_controller->keys[KEY_S].state == PRESSED)
        s_pressed = TRUE;
      if (keyboard_mouse_controller->keys[KEY_A].state == PRESSED)
        a_pressed = TRUE;
      if (keyboard_mouse_controller->keys[KEY_D].state == PRESSED)
        d_pressed = TRUE;
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

  for (i32 ai_num = 0; ai_num < game->current_npcs; ai_num++) {
    r32 steer = game->npcs[ai_num].last_action[0];
    r32 throttle = game->npcs[ai_num].last_action[1];

    if (steer > 1.0f)
      steer = 1.0f;
    if (steer < -1.0f)
      steer = -1.0f;
    if (throttle > 1.0f)
      throttle = 1.0f;
    if (throttle < -1.0f)
      throttle = -1.0f;

    // printf("Steer: %f, Throttle: %f\n", steer, throttle);

    if (ai_num == 0 && EVAL_MODE == TRUE) {
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

    r32 angle = -rotation_speed * (r32)delta_time * steer;

    // Update car heading
    game->npcs[ai_num].heading += angle;

    game->npcs[ai_num].speed += throttle * move_speed * (r32)delta_time;

    // Clamp speed
    if (game->npcs[ai_num].speed > 50.0f * speed_scale)
      game->npcs[ai_num].speed = 50.0f * speed_scale;
    if (game->npcs[ai_num].speed < -20.0f * speed_scale)
      game->npcs[ai_num].speed = -20.0f * speed_scale;

    // Movement + progress based reward
    r32 heading = game->npcs[ai_num].heading;
    vec3 forward_vel = (vec3){.x = (r32)real32_cos(heading), .y = 0.0f, .z = -(r32)real32_sin(heading)};

    // Current marker position
    vec3 marker_pos = game->marker[game->npcs[ai_num].current_marker]->sprite_common.position;

    // Distance BEFORE movement
    r32 dx_before = marker_pos.x - game->npcs[ai_num].position.x;
    r32 dz_before = marker_pos.z - game->npcs[ai_num].position.z;
    r32 dist_before = real32_sqrt(dx_before * dx_before + dz_before * dz_before);

    b8 hit_tree = FALSE;

    ///////////////////////////////////////////////////
    // Start of reward calculation
    ///////////////////////////////////////////////////
    r32 reward = 0.0f;
    r32 speed_before_move = game->npcs[ai_num].speed;
    vec3 prev_pos = game->npcs[ai_num].position;

    //  Move car
    game->npcs[ai_num].position.x += forward_vel.x * game->npcs[ai_num].speed * (r32)delta_time;
    game->npcs[ai_num].position.z += forward_vel.z * game->npcs[ai_num].speed * (r32)delta_time;

    const r32 TREE_RADIUS = 1.75f;
    const r32 TREE_SKIN = 0.20f;  // extra push-out margin
    const r32 BOUNCE_RESTITUTION = 1.5f;
    const r32 MIN_BOUNCE_SPEED = 15.0f;
    const r32 TREE_AVOID_TURN = 0.35f;  // about 20 degrees
    const r32 TREE_SIDE_EPS = 0.05f;

    for (i32 t = 0; t < game->total_trees; t++) {
      vec3 tree_pos = game->trees[t]->sprite_common.position;

      r32 dx = game->npcs[ai_num].position.x - tree_pos.x;
      r32 dz = game->npcs[ai_num].position.z - tree_pos.z;
      r32 dist = real32_sqrt(dx * dx + dz * dz);

      if (dist < TREE_RADIUS) {
        hit_tree = TRUE;

        // Start from pre-move position so we do not stay embedded in the tree
        r32 resolve_x = prev_pos.x;
        r32 resolve_z = prev_pos.z;

        r32 rdx = resolve_x - tree_pos.x;
        r32 rdz = resolve_z - tree_pos.z;
        r32 rdist = real32_sqrt(rdx * rdx + rdz * rdz);

        r32 nx, nz;
        if (rdist > 1e-4f) {
          nx = rdx / rdist;
          nz = rdz / rdist;
        } else if (dist > 1e-4f) {
          nx = dx / dist;
          nz = dz / dist;
        } else {
          nx = -forward_vel.x;
          nz = -forward_vel.z;
        }

        game->npcs[ai_num].position.x = tree_pos.x + nx * (TREE_RADIUS + TREE_SKIN);
        game->npcs[ai_num].position.z = tree_pos.z + nz * (TREE_RADIUS + TREE_SKIN);

        r32 vx = forward_vel.x * speed_before_move;
        r32 vz = forward_vel.z * speed_before_move;
        r32 impact_speed = real32_fabs(vx * nx + vz * nz);

        // Bounce backward
        game->npcs[ai_num].speed = -real32_fmax(MIN_BOUNCE_SPEED, impact_speed * BOUNCE_RESTITUTION);

        // Turn slightly away from the tree so the AI does not keep rehitting it
        r32 current_heading = game->npcs[ai_num].heading;

        r32 rx = -real32_sin(current_heading);
        r32 rz = -real32_cos(current_heading);

        // Tree position in carspace
        r32 to_tree_x = tree_pos.x - game->npcs[ai_num].position.x;
        r32 to_tree_z = tree_pos.z - game->npcs[ai_num].position.z;
        r32 tree_side = to_tree_x * rx + to_tree_z * rz;

        if (real32_fabs(tree_side) < TREE_SIDE_EPS) {
          r32 to_marker_x = marker_pos.x - game->npcs[ai_num].position.x;
          r32 to_marker_z = marker_pos.z - game->npcs[ai_num].position.z;
          tree_side = to_marker_x * rx + to_marker_z * rz;
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
    r32 damping = 2.0f;
    game->npcs[ai_num].speed *= real32_exp((r32)((r64)-damping * delta_time));

    // Update sprite + camera
    game->npcs[ai_num].sprite->sprite_common.position = game->npcs[ai_num].position;
    game->npcs[ai_num].sprite->sprite_common.rotation = sprite_billboard_rotation(game->npcs[ai_num].position, cam_pos);
    game->npcs[ai_num].sprite->sprite_common.frame_layer = car_frame_from_camera(game->npcs[ai_num].heading, steer, game->npcs[ai_num].position, cam_pos);

    //  Distance AFTER movement
    r32 dx_after = marker_pos.x - game->npcs[ai_num].position.x;
    r32 dz_after = marker_pos.z - game->npcs[ai_num].position.z;
    r32 dist_after = real32_sqrt(dx_after * dx_after + dz_after * dz_after);

    // Main dense signal: reward distance reduction
    r32 progress = dist_before - dist_after;
    if (!hit_tree)
      reward += 0.1f * progress;

    const r32 checkpoint_radius = 15.0f;
    if (!hit_tree && dist_after < checkpoint_radius) {
      reward += 2.0f;  // checkpoint bonus

      game->npcs[ai_num].current_marker++;

      if (game->npcs[ai_num].current_marker >= game->total_markers) {
        game->npcs[ai_num].current_marker = 0;
        reward += 5.0f;  // lap bonus
        // done = TRUE;
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
    reward -= 0.02f * real32_fabs(angle);

    // Use the current target marker after checkpoint update so we head towards the next one if we just passed the checkpoint
    vec3 next_marker = game->marker[game->npcs[ai_num].current_marker]->sprite_common.position;

    // World delta to target, basically aiming towards it
    r32 dxw = next_marker.x - game->npcs[ai_num].position.x;
    r32 dzw = next_marker.z - game->npcs[ai_num].position.z;

    r32 fx = -real32_cos(game->npcs[ai_num].heading);
    r32 fz = real32_sin(game->npcs[ai_num].heading);
    r32 rx = -real32_sin(game->npcs[ai_num].heading);
    r32 rz = -real32_cos(game->npcs[ai_num].heading);

    r32 forward_err = dxw * fx + dzw * fz;
    r32 right_err = dxw * rx + dzw * rz;

    // Normalization constants
    const r32 norm = 150.0f;
    const r32 obstacle_norm = 100.0f;

    // Pick the most relevant tree ahead of the car
    b8 found_tree_ahead = FALSE;
    r32 best_forward = obstacle_norm;
    r32 best_right = 0.0f;
    r32 best_score = R32_MAX;

    for (i32 t = 0; t < game->total_trees; t++) {
      vec3 tree_pos = game->trees[t]->sprite_common.position;

      r32 dx = tree_pos.x - game->npcs[ai_num].position.x;
      r32 dz = tree_pos.z - game->npcs[ai_num].position.z;

      r32 forward = dx * fx + dz * fz;
      r32 right = dx * rx + dz * rz;

      if (forward <= 0.0f)
        continue;

      r32 score = forward + 2.0f * real32_fabs(right);
      if (score < best_score) {
        found_tree_ahead = TRUE;
        best_score = score;
        best_forward = forward;
        best_right = right;
      }
    }

    r32 tree_dx = 1.0f;
    r32 tree_dy = 0.0f;
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
      r32 lateral = real32_fabs(tree_dy);
      if (lateral < 0.15f) {
        r32 forward_term = (0.25f - tree_dx) / 0.25f;
        r32 lateral_term = (0.15f - lateral) / 0.15f;
        reward -= 0.15f * forward_term * lateral_term;
      }
    }

    if (!EVAL_MODE) {
      struct Packet {
        r32 dx;       // Next checkpoint x relative to car direction
        r32 dy;       // Next checkpoint y relative to car direction
        r32 speed;    // Car speed
        r32 azimuth;  // Car angle
        r32 tree_dx;  // Next forward facing tree x relative to car direction
        r32 tree_dy;  // Next forward facing tree y relative to car direction
        r32 reward;   // Reward for this step
        i32 done;     // Whether the episode is done
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
      if (start == TRUE) {
        r32 value;
        r32 speed_norm = real32_tanh(game->npcs[ai_num].speed / 120.0f);
        r32 game_state[7] = {forward_err / norm, right_err / norm, speed_norm, real32_sin(heading), real32_cos(heading), tree_dx, tree_dy};
        ac_forward(&(game->npcs[ai_num].model), game_state, game->npcs[ai_num].last_action, &value);
      }
    }
  }

  if (game->current_npcs > 0) {
    i32 follow = game->camera_current_follow_kart;

    if (follow < 0)
      follow = 0;
    if (follow >= game->current_npcs)
      follow = game->current_npcs - 1;

    game->player.look_at_pos = (vec3d){.x = (r64)game->npcs[follow].position.x, .y = (r64)game->npcs[follow].position.y, .z = (r64)game->npcs[follow].position.z};
    game->player.camera.look_at_azimuth = -(r64)game->npcs[follow].heading;
  }

  player_update(&(game->player), input_manager_get_controller_actions(input_manager), input_manager_get_controller_action_list_length(input_manager));

  // Make this always facing toward the camera
  for (i32 marker_num = 0; marker_num < game->total_markers; marker_num++) {
    mat4 marker_rotation = mat4_rotate(MAT4_IDENTITY, -(r32)R32_PI / 2.0f, (vec3){.x = 0.5f, .y = 0.0f, .z = 0.0f});
    marker_rotation = mat4_rotate(marker_rotation, (r32)R32_PI / 2.0f, (vec3){.x = 0.0f, .y = 1.0f, .z = 0.0f});
    game->marker[marker_num]->sprite_common.rotation = mat4_to_quaternion(mat4_rotate(marker_rotation, (r32)-game->player.camera.look_at_azimuth, (vec3){.x = 0.0f, .y = 1.0f, .z = 0.0f}));
    // TODO: Commented out temporarily because we just want to hide markers for now
    // if (EVAL_MODE)
    game->marker[marker_num]->sprite_common.position.y = -10000.0f;
  }
  mat4 flag1_rotation = mat4_rotate(MAT4_IDENTITY, -(r32)R32_PI / 2.0f, (vec3){.x = 0.5f, .y = 0.0f, .z = 0.0f});
  flag1_rotation = mat4_rotate(flag1_rotation, (r32)R32_PI / 2.0f, (vec3){.x = 0.0f, .y = 1.0f, .z = 0.0f});
  game->flag1->sprite_common.rotation = mat4_to_quaternion(mat4_rotate(flag1_rotation, (r32)-game->player.camera.look_at_azimuth, (vec3){.x = 0.0f, .y = 1.0f, .z = 0.0f}));
  mat4 flag2_rotation = mat4_rotate(MAT4_IDENTITY, -(r32)R32_PI / 2.0f, (vec3){.x = 0.5f, .y = 0.0f, .z = 0.0f});
  flag2_rotation = mat4_rotate(flag2_rotation, (r32)R32_PI / 2.0f, (vec3){.x = 0.0f, .y = 1.0f, .z = 0.0f});
  game->flag2->sprite_common.rotation = mat4_to_quaternion(mat4_rotate(flag2_rotation, (r32)-game->player.camera.look_at_azimuth, (vec3){.x = 0.0f, .y = 1.0f, .z = 0.0f}));
}

void game_render(struct Game* game, struct Mana* mana, r64 delta_time) {
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

      window->should_resize = FALSE;
    }

    // Diffuse sun directional light
    r32 sun_intensity = 4.0f;
    vec3 sun_dir = vec3_normalize((vec3){.x = 0.35f, .y = -0.90f, .z = 0.25f});
    vec4 full_dir = (vec4){.x = sun_dir.x, .y = sun_dir.y, .z = sun_dir.z, .w = 0.15f};
    vec4 diffuse_color = (vec4){.x = 1.0f * sun_intensity, .y = 0.96f * sun_intensity, .z = 0.86f * sun_intensity, .w = 0.0f};
    // Soft sky fill, not white
    vec4 ambient_color = (vec4){.x = 0.10f, .y = 0.14f, .z = 0.18f, .w = 0.0f};
    vec4 specular_light = (vec4){.x = 1.0f, .y = 0.98f, .z = 0.95f, .w = 0.0f};

    model_update_uniforms(game->test_model, api_common, window->gbuffer, camera_get_pos(&(game->player.camera)), full_dir, diffuse_color, ambient_color, specular_light);
    model_update_uniforms(game->test_static_model, api_common, window->gbuffer, camera_get_pos(&(game->player.camera)), full_dir, diffuse_color, ambient_color, specular_light);
    model_update_uniforms(game->coin_model, api_common, window->gbuffer, camera_get_pos(&(game->player.camera)), full_dir, diffuse_color, ambient_color, specular_light);
    model_update_uniforms(game->track_model, api_common, window->gbuffer, camera_get_pos(&(game->player.camera)), full_dir, diffuse_color, ambient_color, specular_light);

    water_update_uniforms(&(game->water), api_common, &(window->gbuffer->gbuffer_common), window->renderer.renderer_settings.width, window->renderer.renderer_settings.height);

    sprite_manager_update(&(game->sprite_manager), delta_time);
    sprite_manager_update_uniforms(&(game->sprite_manager), api_common, &(window->gbuffer->gbuffer_common));

    // GBuffer, only texture that needs to be multisampled
    gbuffer_start(window->gbuffer, &(window->swap_chain->swap_chain_common), window->renderer.renderer_settings.msaa_samples);

    water_render(&(game->water), &(window->gbuffer->gbuffer_common));

    model_render(game->test_model, window->gbuffer, delta_time);
    model_render(game->test_static_model, window->gbuffer, delta_time);
    model_render(game->coin_model, window->gbuffer, delta_time);
    model_render(game->track_model, window->gbuffer, delta_time);

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
