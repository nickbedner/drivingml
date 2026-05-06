#define WIN32_LEAN_AND_MEAN
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <Windows.h>

////////

#include "drivingml/game.h"

void game_init(struct Game* game, struct Mana* mana, struct Window* window) {
  game->window = window;

  player_init(&(game->player), 1, game->window->renderer.renderer_settings.height);

  game->previous_reward = 0.0f;

  game->audio_volome = 0.1f;

  if (DEPLOY_MODE || DRIVE_OVERRIDE) {
    audio_engine_start(&(game->audio_engine));
    char fart_path[MAX_LENGTH_OF_PATH] = {0};
    snprintf(fart_path, MAX_LENGTH_OF_PATH, "%s/audio/fart.wav", mana->api.api_common.asset_directory);
    load_audio(fart_path, &(game->raw_fart_sfx));
    audio_prepare_clip(&game->raw_fart_sfx, &game->fart_sfx);
    audio_play_sfx(&(game->audio_engine), &game->fart_sfx, game->audio_volome);

    char music_path[MAX_LENGTH_OF_PATH] = {0};
    snprintf(music_path, MAX_LENGTH_OF_PATH, "%s/audio/Koopa Troopa Beach.wav", mana->api.api_common.asset_directory);
    load_audio(music_path, &(game->raw_music));
    audio_prepare_clip(&game->raw_music, &game->music_sfx);
    audio_play_music(&(game->audio_engine), &game->music_sfx, game->audio_volome, TRUE);
  }

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
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/boats/boat1.png", TRUE);
  sprite_texture_settings = (struct TextureSettings){.filter_type = FILTER_ANISOTROPIC, .mode_type = MODE_CLAMP_TO_EDGE, .format_type = FORMAT_R8G8B8A8_UNORM, .mip_type = MIP_GENERATE, .mip_count = 5, .premultiplied_alpha = TRUE, .max_anisotropy = 16.0f};
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/track.png", TRUE);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/circuit.png", TRUE);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/startfinish.png", TRUE);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/cloud.png", TRUE);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/aero.png", TRUE);
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
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/models/plane/diffuse.png", TRUE);
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/models/plane/normal.png", TRUE);
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

  const char* tentacle_frames[] = {
      "/textures/tentacle/tile000.png",
      "/textures/tentacle/tile001.png",
      "/textures/tentacle/tile002.png",
      "/textures/tentacle/tile003.png",
      "/textures/tentacle/tile004.png",
      "/textures/tentacle/tile005.png",
      "/textures/tentacle/tile006.png",
      "/textures/tentacle/tile007.png"};
  texture_manager_add_array(&(game->texture_manager), &(mana->api.api_common), sprite_texture_settings, "/textures/tentacle", tentacle_frames, 8);

  sprite_manager_init(&(game->sprite_manager), &(game->texture_manager), &(mana->api.api_common), window->renderer.renderer_settings.width, window->renderer.renderer_settings.height, window->swap_chain->swap_chain_common.supersample_scale, &(window->gbuffer->gbuffer_common), window->renderer.renderer_settings.msaa_samples, 128);

  water_shader_init(&(game->water_shader), &(mana->api.api_common), window->renderer.renderer_settings.width, window->renderer.renderer_settings.height, window->swap_chain->swap_chain_common.supersample_scale, &(window->gbuffer->gbuffer_common), window->renderer.renderer_settings.msaa_samples, 3);
  water_init(&(game->water), &(mana->api.api_common), &(game->water_shader.shader), texture_manager_get(game->sprite_manager.sprite_manager_common.texture_manager, "/textures/waterm1.png"));
  game->water.water_common.position = (vec3){.x = 0.0f, .y = -5.0f, .z = 0.0f};
  game->water.water_common.scale = (vec3){.x = 2048.0f * 64.0f, .y = 1.0f, .z = 2048.0f * 64.0f};

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

  struct ModelSettings model_plane_settings = (struct ModelSettings){
      .path = "./assets/models/plane/plane.dae",
      .shader = &(game->model_cache.model_static_shader.shader),
      .diffuse_texture = texture_manager_get(&(game->texture_manager), "/models/plane/diffuse.png"),
      .normal_texture = texture_manager_get(&(game->texture_manager), "/models/plane/normal.png"),
      .metallic_texture = texture_manager_get(&(game->texture_manager), "/models/track/metallic.png"),
      .roughness_texture = texture_manager_get(&(game->texture_manager), "/models/track/roughness.png"),
      .ao_texture = texture_manager_get(&(game->texture_manager), "/models/track/ao.png"),
      5};
  model_cache_add(&(game->model_cache), &(mana->api.api_common), &model_plane_settings, 3, FALSE);
  game->game_map.plane_model = model_cache_get(&(game->model_cache), &(mana->api.api_common), "./assets/models/plane/plane.dae");
  game->game_map.plane_model->model_common.position = (vec3){.x = 0.0f, .y = -50.0f, .z = 0.0f};
  game->game_map.plane_model->model_common.scale = (vec3){.x = 100000.0f, .y = 1.0f, .z = 100000.0f};

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
  model_cache_add(&(game->model_cache), &(mana->api.api_common), &coin_settings, 4, FALSE);
  game->coin_model = model_cache_get(&(game->model_cache), &(mana->api.api_common), "./assets/models/Watermelon/watermelon.dae");
  game->coin_model->model_common.scale = (vec3){.x = 1.5f, .y = 1.5f, .z = 1.5f};
  game->coin_model->model_common.position = (vec3){.x = 15.0f, .y = 4.0f, .z = 0.0f};

  game->cloud1 = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), "/textures/cloud.png");
  game->cloud1->sprite_common.position = (vec3){.x = 2000.0f, .y = 1500.0f, .z = -10000.0f};
  game->cloud1->sprite_common.scale = (vec3){.x = 2000.0f, .y = 2000.0f, .z = 1.0f};

  game->cloud2 = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), "/textures/cloud.png");
  game->cloud2->sprite_common.position = (vec3){.x = 4250.0f, .y = 750.0f, .z = -9500.0f};
  game->cloud2->sprite_common.scale = (vec3){.x = 1000.0f, .y = 1000.0f, .z = 1.0f};

  game->aero = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), "/textures/aero.png");
  game->aero->sprite_common.position = (vec3){.x = -500.0f, .y = 750.0f, .z = -7500.0f};
  game->aero->sprite_common.scale = (vec3){.x = 75.0f, .y = 7500.0f, .z = 1.0f};
  mat4 rot = mat4_rotate(MAT4_IDENTITY, (r32)R32_PI / 2.0f, (vec3){.x = 1.0f, .y = 0.0f, .z = 0.0f});
  // rot = mat4_rotate(rot, (r32)R32_PI / 2.0f, (vec3){.x = 0.0f, .y = 0.0f, .z = 0.0f});
  game->aero->sprite_common.rotation = mat4_to_quaternion(rot);

  game->boat1 = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), "/textures/boats/boat1.png");
  game->boat1->sprite_common.position = (vec3){.x = 45.0f, .y = 16.0f, .z = 50.0f};
  game->boat1->sprite_common.scale = (vec3){.x = 25.0f, .y = 25.0f, .z = 1.0f};

  game->tentacle.sprite = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), "/textures/tentacle");
  game->tentacle.sprite->sprite_common.position = (vec3){.x = 30.0f, .y = 3.25f, .z = -138.0f};
  game->tentacle.sprite->sprite_common.scale = (vec3){.x = 10.0f, .y = 10.0f, .z = 1.0f};
  game->tentacle.frame = 0;
  game->tentacle.max_frames = 8;
  game->tentacle.accum = 0.0f;
  game->tentacle.accum_limit = 0.5f;

  if (!DEPLOY_MODE && !DRIVE_OVERRIDE) {
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

  char path[MAX_LENGTH_OF_PATH] = {0};
  snprintf(path, MAX_LENGTH_OF_PATH, "%s/maps.xml", mana->api.api_common.asset_directory);
  load_map_from_xml(game, mana, &(game->game_map), path, "track0");

  game->timer = 0;
  game->start_timer = 0;
  game->state_update_accum = 0.0;
  game->camera_current_follow_kart = 0;
  if (DEPLOY_MODE)
    game->current_npcs += MAX_NPCS;
  else
    game->current_npcs += 1;
  for (i32 npc_num = 0; npc_num < game->current_npcs; npc_num++) {
    if (npc_num == 0) {
      if (DEPLOY_MODE == FALSE)
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
    } else {
      game->npcs[npc_num].sprite = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), "/textures/aikartgreen");
      load_ac_model("checkpoints/ac_weights500.bin", &(game->npcs[npc_num].model));
    }
    // Forced override for testing
    if (npc_num < 5)
      load_ac_model("checkpoints/ac_weightsh.bin", &(game->npcs[npc_num].model));
    else
      load_ac_model("checkpoints/ac_weightsl.bin", &(game->npcs[npc_num].model));

    game->npcs[npc_num].speed = 0.0f;

    const r32 GRID_FORWARD_SPACING = 17.0f;
    const r32 GRID_LANE_SPACING = 8.0f;

    r32 h = game->game_map.start_heading;

    vec3 forward = {.x = real32_cos(h), .y = 0.0f, .z = -real32_sin(h)};
    vec3 right = {.x = real32_sin(h), .y = 0.0f, .z = real32_cos(h)};

    i32 row = npc_num / 2;
    i32 col = npc_num % 2;

    const r32 LEFT_COLUMN_STAGGER = 3.0f;

    r32 forward_offset = (r32)row * GRID_FORWARD_SPACING;
    r32 side_offset = (r32)col * GRID_LANE_SPACING;

    if (col == 0)
      forward_offset += LEFT_COLUMN_STAGGER;

    game->npcs[npc_num].position = (vec3){.x = game->game_map.start_pos.x + forward.x * forward_offset + right.x * side_offset, .y = game->game_map.start_pos.y, .z = game->game_map.start_pos.z + forward.z * forward_offset + right.z * side_offset};

    game->npcs[npc_num].sprite->sprite_common.position = game->npcs[npc_num].position;
    game->npcs[npc_num].sprite->sprite_common.scale = (vec3){.x = 5.0f, .y = 5.0f, .z = 0.0f};
    game->npcs[npc_num].heading = game->game_map.start_heading;
    game->npcs[npc_num].last_marker = -1;
    i32 start_marker_index = game_map_marker_index_from_id(&game->game_map, 0);
    if (start_marker_index < 0)
      start_marker_index = 0;
    game->npcs[npc_num].current_marker = start_marker_index;
    game->npcs[npc_num].risk_preference = AI_RISK_PREFERENCE;
    game->npcs[npc_num].last_action[0] = 0.0f;
    game->npcs[npc_num].last_action[1] = 0.0f;
    game->npcs[npc_num].prev_y = game->npcs[npc_num].position.y;
  }
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
  if (DEPLOY_MODE || DRIVE_OVERRIDE) {
    audio_engine_stop(&(game->audio_engine));
    audio_free_clip_f32(&(game->fart_sfx));
    audio_free_clip(&(game->raw_fart_sfx));
    audio_free_clip_f32(&(game->music_sfx));
    audio_free_clip(&(game->raw_music));
  }
}

void game_update(struct Game* game, struct Mana* mana, r64 delta_time) {
  struct Window* window = game->window;
  struct InputManager* input_manager = &window->input_manager;

  const r64 state_dt = 1.0 / 60.0;
  game->state_update_accum += delta_time;
  b8 update_state = FALSE;
  if (game->state_update_accum >= state_dt) {
    update_state = TRUE;
    game->state_update_accum = 0.0;
  }

  r32 player_steer = 0.0f;
  r32 player_throttle = 0.0f;

  for (size_t controller_num = 0; controller_num < input_manager->controllers.size; controller_num++) {
    struct Controller* controller = (struct Controller*)array_list_get(&(input_manager->controllers), controller_num);
    if (controller->controller_common.controller_type == CONTROLLER_XBOX) {
      struct XboxController* xbox_controller = &(controller->controller_common.xbox_controller);

      if (xbox_controller->buttons[XBOX_CONTROLLER_ACTION_DPAD_LEFT].pushed) {
        game->camera_current_follow_kart--;
        if (game->camera_current_follow_kart < 0)
          game->camera_current_follow_kart = game->current_npcs - 1;
      }
      if (xbox_controller->buttons[XBOX_CONTROLLER_ACTION_DPAD_RIGHT].pushed) {
        game->camera_current_follow_kart++;
        if (game->camera_current_follow_kart >= game->current_npcs)
          game->camera_current_follow_kart = 0;
      }

      if (xbox_controller->buttons[XBOX_CONTROLLER_ACTION_A].state == PRESSED)
        player_throttle = 1.0f;
      if (xbox_controller->buttons[XBOX_CONTROLLER_ACTION_B].state == PRESSED)
        player_throttle = -1.0f;
      player_steer = xbox_controller->left_x;
    }
  }

  // Hack to make sure gamepad joystick can't overwrite keyboard values
  for (size_t controller_num = 0; controller_num < input_manager->controllers.size; controller_num++) {
    struct Controller* controller = (struct Controller*)array_list_get(&(input_manager->controllers), controller_num);
    if (controller->controller_common.controller_type == CONTROLLER_KEYBOARD_MOUSE) {
      struct KeyboardMouseController* keyboard_mouse_controller = &(controller->controller_common.keyboard_mouse_controller);

      if (keyboard_mouse_controller->keys[KEY_LEFT].pushed) {
        game->camera_current_follow_kart--;
        if (game->camera_current_follow_kart < 0)
          game->camera_current_follow_kart = game->current_npcs - 1;
      }
      if (keyboard_mouse_controller->keys[KEY_RIGHT].pushed) {
        game->camera_current_follow_kart++;
        if (game->camera_current_follow_kart >= game->current_npcs)
          game->camera_current_follow_kart = 0;
      }

      if (keyboard_mouse_controller->keys[KEY_W].state == PRESSED)
        player_throttle = 1.0f;
      if (keyboard_mouse_controller->keys[KEY_S].state == PRESSED)
        player_throttle = -1.0f;
      if (keyboard_mouse_controller->keys[KEY_A].state == PRESSED)
        player_steer = -1.0f;
      if (keyboard_mouse_controller->keys[KEY_D].state == PRESSED)
        player_steer = 1.0f;
    }
  }

  if (update_state || (!DEPLOY_MODE && !DRIVE_OVERRIDE)) {
    float sim_delta_time = 0.01666666666;
    b8 done = FALSE;

    //// Delta time clamping, good for ML but not so good for game?
    // if (delta_time > 0.05)
    //   delta_time = 0.05;

    // Hardcoded to episode length of 45 seconds before timeout
    // If I can complete course in about 30 secnds then AI should have 1.3x to 1.8x buffer
    game->timer++;
    if (game->timer > 2700 && !DEPLOY_MODE) {
      printf("Episode timed out\n");
      game->timer = 0;
      done = TRUE;
    }

    game->start_timer++;
    b8 start = FALSE;
    if (game->start_timer > 180)
      start = TRUE;

    // const r32 speed_scale = 144.0f / 60.0f;  // 2.4
    // r32 move_speed = 30.0f * speed_scale;
    // r32 rotation_speed = 1.5f * speed_scale;
    r32 move_speed = 125.0f;
    r32 rotation_speed = 4.0f;
    vec3d cam_pos = camera_get_pos(&game->player.camera);

    // game->cloud->sprite_common.rotation = sprite_billboard_rotation(game->boat1->sprite_common.position, cam_pos);

    game->boat1->sprite_common.rotation = sprite_billboard_rotation(game->boat1->sprite_common.position, cam_pos);
    game->boat1->sprite_common.position.z -= 2.5f * (r32)sim_delta_time;

    game->aero->sprite_common.position.z += 50.0f * (r32)sim_delta_time;

    for (i32 t = 0; t < game->total_trees; t++)
      game->game_map.trees[t]->sprite_common.rotation = sprite_billboard_rotation(game->game_map.trees[t]->sprite_common.position, cam_pos);

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

      if (ai_num == 0 && DRIVE_OVERRIDE == TRUE) {
        throttle = player_throttle;
        steer = player_steer;
      }

      r32 angle = -rotation_speed * (r32)sim_delta_time * steer;

      // Update car heading
      game->npcs[ai_num].heading += angle;

      game->npcs[ai_num].speed += throttle * move_speed * (r32)sim_delta_time;

      // Clamp speed
      if (game->npcs[ai_num].speed > 500.0f)
        game->npcs[ai_num].speed = 500.0f;
      if (game->npcs[ai_num].speed < -50.0f)
        game->npcs[ai_num].speed = -50.0f;

      // Movement + progress based reward
      r32 heading = game->npcs[ai_num].heading;
      vec3 forward_vel = (vec3){.x = (r32)real32_cos(heading), .y = 0.0f, .z = -(r32)real32_sin(heading)};

      // Current marker position
      vec3 marker_pos = game->game_map.markers[game->npcs[ai_num].current_marker].sprite->sprite_common.position;

      // Distance BEFORE movement
      r32 dx_before = marker_pos.x - game->npcs[ai_num].position.x;
      r32 dz_before = marker_pos.z - game->npcs[ai_num].position.z;
      r32 dist_before = real32_sqrt(dx_before * dx_before + dz_before * dz_before);

      b8 hit_tree = FALSE;

      ///////////////////////////////////////////////////
      // Start of reward calculation
      ///////////////////////////////////////////////////
      r32 reward = 0.0f;
      //  Move car
      game->npcs[ai_num].position.x += forward_vel.x * game->npcs[ai_num].speed * (r32)sim_delta_time;
      game->npcs[ai_num].position.z += forward_vel.z * game->npcs[ai_num].speed * (r32)sim_delta_time;

      // For following track
      r32 track_y = 0.0f;
      i32 surface_type = TRACK_SURFACE_UNKNOWN;
      if (track_get_height_at(game->game_map.track_model, game->npcs[ai_num].position.x, game->npcs[ai_num].position.z, &track_y, &surface_type)) {
        const r32 kart_ground_offset = 2.75f;
        game->npcs[ai_num].position.y = track_y + kart_ground_offset;

        // if (ai_num == 0 && DEPLOY_MODE == TRUE)
        //   printf("kart %d is on %s\n", ai_num, track_surface_type_name(surface_type));
      }

      heading = game->npcs[ai_num].heading;

      // Apply damping
      r32 damping = 2.0f;
      game->npcs[ai_num].speed *= real32_exp((r32)((r64)-damping * sim_delta_time));

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
        reward += 2.0f;

        i32 completed_marker = game->npcs[ai_num].current_marker;
        game->npcs[ai_num].last_marker = completed_marker;

        i32 next_marker = choose_next_marker_by_risk(&game->game_map, completed_marker, game->npcs[ai_num].risk_preference);

        if (next_marker >= 0) {
          game->npcs[ai_num].current_marker = next_marker;

          // Lap bonus when route points back to marker id 0.
          if (game->game_map.markers[next_marker].id == 0 &&
              game->game_map.markers[completed_marker].id != 0) {
            reward += 5.0f;
          }
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
      vec3 next_marker = game->game_map.markers[game->npcs[ai_num].current_marker].sprite->sprite_common.position;

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
        vec3 tree_pos = game->game_map.trees[t]->sprite_common.position;

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

      struct TargetOption target_options[2];
      get_current_target_options(&game->game_map, &game->npcs[ai_num], target_options);

      r32 option0_dx = 0.0f;
      r32 option0_dy = 0.0f;
      r32 option0_risk = 0.0f;
      r32 option1_dx = 0.0f;
      r32 option1_dy = 0.0f;
      r32 option1_risk = 0.0f;

      marker_option_to_local_features(&game->game_map, &game->npcs[ai_num], target_options[0], norm, &option0_dx, &option0_dy, &option0_risk);
      marker_option_to_local_features(&game->game_map, &game->npcs[ai_num], target_options[1], norm, &option1_dx, &option1_dy, &option1_risk);

      if (!DEPLOY_MODE && !DRIVE_OVERRIDE) {
        struct Packet {
          r32 option0_dx;
          r32 option0_dy;
          r32 option0_risk;

          r32 option1_dx;
          r32 option1_dy;
          r32 option1_risk;

          r32 risk_preference;

          r32 speed;
          r32 azimuth;

          r32 tree_dx;
          r32 tree_dy;

          r32 reward;
          i32 done;
        };

        struct Packet packet;

        packet.option0_dx = option0_dx;
        packet.option0_dy = option0_dy;
        packet.option0_risk = option0_risk;

        packet.option1_dx = option1_dx;
        packet.option1_dy = option1_dy;
        packet.option1_risk = option1_risk;

        packet.risk_preference = game->npcs[ai_num].risk_preference;

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
          game->npcs[ai_num].position = game->game_map.start_pos;
          game->timer = 0;
          r32 reset_track_y = 0.0f;
          i32 reset_surface_type = TRACK_SURFACE_UNKNOWN;

          if (track_get_height_at(game->game_map.track_model, game->npcs[ai_num].position.x, game->npcs[ai_num].position.z, &reset_track_y, &reset_surface_type))
            game->npcs[ai_num].position.y = reset_track_y + 0.75f;

          game->npcs[ai_num].sprite->sprite_common.position = game->npcs[ai_num].position;
          game->npcs[ai_num].sprite->sprite_common.scale = (vec3){.x = 5.0f, .y = 5.0f, .z = 0.0f};
          game->npcs[ai_num].heading = game->game_map.start_heading;
          game->npcs[ai_num].sprite->sprite_common.rotation = sprite_billboard_rotation(game->npcs[ai_num].position, cam_pos);
          game->npcs[ai_num].sprite->sprite_common.frame_layer = car_frame_from_camera(game->npcs[ai_num].heading, steer, game->npcs[ai_num].position, cam_pos);

          game->npcs[ai_num].last_action[0] = 0.0f;
          game->npcs[ai_num].last_action[1] = 0.0f;
          game->npcs[ai_num].prev_y = game->npcs[ai_num].position.y;

          game->npcs[ai_num].last_marker = -1;

          i32 reset_marker_index = game_map_marker_index_from_id(&game->game_map, 0);
          if (reset_marker_index < 0)
            reset_marker_index = 0;

          game->npcs[ai_num].current_marker = reset_marker_index;
        }
      } else {
        if (start == TRUE) {
          r32 value;
          r32 speed_norm = real32_tanh(game->npcs[ai_num].speed / 120.0f);
          r32 game_state[12] = {option0_dx, option0_dy, option0_risk, option1_dx, option1_dy, option1_risk, game->npcs[ai_num].risk_preference, speed_norm, real32_sin(heading), real32_cos(heading), tree_dx, tree_dy};
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

      game->player.look_at_pos = (vec3d){.x = (r64)game->npcs[follow].position.x, .y = (r64)game->npcs[follow].position.y + 1.5, .z = (r64)game->npcs[follow].position.z};
      game->player.look_at_azimuth = -(r64)game->npcs[follow].heading;
      game->player.look_at_elevation = -R64_PI / 32.0;
      game->player.camera.look_at_elevation = 1.25;
    }

    player_update(&(game->player), input_manager_get_controller_actions(input_manager), input_manager_get_controller_action_list_length(input_manager));

    for (i32 marker_num = 0; marker_num < game->total_markers; marker_num++) {
      struct Sprite* marker_sprite = game->game_map.markers[marker_num].sprite;

      if (!marker_sprite)
        continue;

      marker_sprite->sprite_common.rotation = sprite_billboard_rotation(marker_sprite->sprite_common.position, cam_pos);

      if (DEPLOY_MODE)
        marker_sprite->sprite_common.position.y = -10000.0f;
    }

    game->tentacle.accum += (r32)sim_delta_time;
    if (game->tentacle.accum > game->tentacle.accum_limit) {
      game->tentacle.frame = (game->tentacle.frame + 1) % game->tentacle.max_frames;
      game->tentacle.accum = 0.0f;
      game->tentacle.sprite->sprite_common.frame_layer = game->tentacle.frame;
      if (game->tentacle.frame == 4) {
        if (DEPLOY_MODE || DRIVE_OVERRIDE)
          audio_play_sfx(&(game->audio_engine), &game->fart_sfx, game->audio_volome);
      }
    }
    // game->tentacle.sprite->sprite_common.rotation = sprite_billboard_rotation(game->tentacle.sprite->sprite_common.position, cam_pos);
  }
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
      shader_resize(&(game->model_cache.model_shader.shader), api_common, window->renderer.renderer_settings.width, window->renderer.renderer_settings.height, window->swap_chain->swap_chain_common.supersample_scale);
      shader_resize(&(game->model_cache.model_static_shader.shader), api_common, window->renderer.renderer_settings.width, window->renderer.renderer_settings.height, window->swap_chain->swap_chain_common.supersample_scale);

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
    model_update_uniforms(game->game_map.track_model, api_common, window->gbuffer, camera_get_pos(&(game->player.camera)), full_dir, diffuse_color, ambient_color, specular_light);
    model_update_uniforms(game->game_map.plane_model, api_common, window->gbuffer, camera_get_pos(&(game->player.camera)), full_dir, diffuse_color, ambient_color, specular_light);

    water_update_uniforms(&(game->water), api_common, &(window->gbuffer->gbuffer_common), window->renderer.renderer_settings.width, window->renderer.renderer_settings.height);

    sprite_manager_update(&(game->sprite_manager), delta_time);
    sprite_manager_update_uniforms(&(game->sprite_manager), api_common, &(window->gbuffer->gbuffer_common));

    // GBuffer, only texture that needs to be multisampled
    gbuffer_start(window->gbuffer, &(window->swap_chain->swap_chain_common), window->renderer.renderer_settings.msaa_samples);

    model_render(game->test_model, window->gbuffer, delta_time);
    model_render(game->test_static_model, window->gbuffer, delta_time);
    model_render(game->coin_model, window->gbuffer, delta_time);
    model_render(game->game_map.track_model, window->gbuffer, delta_time);
    model_render(game->game_map.plane_model, window->gbuffer, delta_time);

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
