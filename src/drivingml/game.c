#include "drivingml/game.h"

void game_init(struct Game* game, struct Mana* mana, struct Window* window) {
  game->window = window;

  player_init(&(game->player), 1, game->window->renderer.renderer_settings.height);

  struct TextureSettings sprite_texture_settings = {FILTER_NEAREST, MODE_CLAMP_TO_EDGE, FORMAT_R8G8B8A8_UNORM, true, true};
  texture_manager_init(&(game->texture_manager), &(mana->api.api_common));
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), &sprite_texture_settings, L"/textures/spritesheet.png");
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), &sprite_texture_settings, L"/textures/water.png");
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), &sprite_texture_settings, L"/textures/map.png");
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), &sprite_texture_settings, L"/textures/mario.png");
  texture_manager_add(&(game->texture_manager), &(mana->api.api_common), &sprite_texture_settings, L"/textures/whispy.png");
  //
  // texture_manager_add(&(game->texture_manager), &(mana->api.api_common), &sprite_texture_settings, L"/textures/pbr/rust/albedo.png");
  // texture_manager_add(&(game->texture_manager), &(mana->api.api_common), &sprite_texture_settings, L"/textures/pbr/rust/normal.png");
  // texture_manager_add(&(game->texture_manager), &(mana->api.api_common), &sprite_texture_settings, L"/textures/pbr/rust/metallic.png");
  // texture_manager_add(&(game->texture_manager), &(mana->api.api_common), &sprite_texture_settings, L"/textures/pbr/rust/roughness.png");
  // texture_manager_add(&(game->texture_manager), &(mana->api.api_common), &sprite_texture_settings, L"/textures/pbr/rust/ao.png");

  sprite_manager_init(&(game->sprite_manager), &(game->texture_manager), &(mana->api.api_common), window->renderer.renderer_settings.width, window->renderer.renderer_settings.height, window->swap_chain->swap_chain_common.supersample_scale, &(window->gbuffer->gbuffer_common), window->renderer.renderer_settings.msaa_samples, 128);

  game->mario_speed = 0.0f;

  game->water = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), L"/textures/water.png");
  // game->sprite->sprite_common.position = (vec3){.x = 0, .y = 15.0f, .z = 0};
  game->water->sprite_common.position = (vec3){.x = 0, .y = 0.0f, .z = -10};
  game->water->sprite_common.scale = (vec3){.x = 30.0f, .y = 30.0f, .z = 0.0f};
  mat4 water_rotation = mat4_rotate(MAT4_IDENTITY, -M_PI, (vec3){.x = 1, .y = 0, .z = 0});
  //  sprite_rotation.m00 = M_PI / 3.25f;
  game->water->sprite_common.rotation = mat4_to_quaternion(water_rotation);

  game->map = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), L"/textures/map.png");
  // game->sprite->sprite_common.position = (vec3){.x = 0, .y = 15.0f, .z = 0};
  game->map->sprite_common.position = (vec3){.x = 0, .y = 0.0f, .z = 0};
  game->map->sprite_common.scale = (vec3){.x = 20.0f, .y = 20.0f, .z = 0.0f};
  mat4 map_rotation = mat4_rotate(MAT4_IDENTITY, -M_PI, (vec3){.x = 1, .y = 0, .z = 0});
  //  sprite_rotation.m00 = M_PI / 3.25f;
  game->map->sprite_common.rotation = mat4_to_quaternion(map_rotation);

  for (int whispy_num = 0; whispy_num < 5; whispy_num++) {
    game->whispy[whispy_num] = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), L"/textures/whispy.png");
    if (whispy_num % 2 == 0)
      game->whispy[whispy_num]->sprite_common.position = (vec3){.x = 20.0f + whispy_num * 15.0f, .y = 20.0f + (whispy_num * 15.0f), .z = 3.0f};
    else
      game->whispy[whispy_num]->sprite_common.position = (vec3){.x = 20.0f + whispy_num * 15.0f, .y = 20.0f - (whispy_num * 5.0f), .z = 3.0f};
    game->whispy[whispy_num]->sprite_common.scale = (vec3){.x = 1.0f, .y = 1.0f, .z = 0.0f};
    mat4 whispy_rotation = mat4_rotate(MAT4_IDENTITY, -M_PI / 2, (vec3){.x = 0.5, .y = 0.0, .z = 0.0});
    whispy_rotation = mat4_rotate(whispy_rotation, M_PI, (vec3){.x = 0.0, .y = 1.0, .z = 0.0});
    game->whispy[whispy_num]->sprite_common.rotation = mat4_to_quaternion(whispy_rotation);
  }

  game->mario = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), L"/textures/mario.png");
  game->mario_position = (vec3){.x = -1.0f, .y = -75.0f, .z = 0.75};
  game->mario->sprite_common.position = game->mario_position;
  game->mario->sprite_common.scale = (vec3){.x = 5.0f, .y = 5.0f, .z = 0.0f};
  mat4 mario_rotation = mat4_rotate(MAT4_IDENTITY, -M_PI / 2, (vec3){.x = 0.5, .y = 0.0, .z = 0.0});
  mario_rotation = mat4_rotate(mario_rotation, M_PI / 2, (vec3){.x = 0.0, .y = 1.0, .z = 0.0});
  game->mario->sprite_common.rotation = mat4_to_quaternion(mario_rotation);

  // game->sprite->sprite_common.rotation = quaternion_rotate_by_vector((quat){.data[0] = 0, .data[1] = 0, .data[2] = 0, .data[3] = 1.0f}, (vec3){.x = 0, .y = 1, .z = 0});
  //  game->sprite->sprite_common.rotation = (quat){.data[0] = M_PI / 3.25f, .data[1] = 0, .data[2] = 0.0, .data[3] = 1.0f};
  //      sprite->sprite_common.position = (vec3){.x = 0, .y = 0.0f, .z = 0};
  //      sprite->sprite_common.rotation = (quat){.data[0] = 0, .data[1] = 0, .data[2] = 0, .data[3] = 1.0f};

  // struct SpriteAnimation *sprite_animation = sprite_manager_add_sprite_animation(&(game->sprite_manager), &(mana->api.api_common), L"/textures/spritesheet.png", 4, 0.5f, 200);
  // sprite_animation->sprite_animation_common.position = (vec3){.x = 0, .y = 15.0f, .z = 0};
  // sprite_animation->sprite_animation_common.rotation = (quat){.data[0] = M_PI / 3.25f, .data[1] = 0, .data[2] = 0, .data[3] = 1.0f};
  // sprite_animation->sprite_animation_common.direction = SPRITE_ANIMATION_FORWARD;

  player_init(&(game->player), 1, game->window->renderer.renderer_settings.height);

  game->player.player_controller.pos = (vec3d){.x = 0, .y = -10, .z = 5};

  game->player.camera.look_at_azimuth += M_PI / 2;

  // struct Sprite *sprite = calloc(1, sizeof(struct Sprite));

  // for (size_t loop_num = 1; loop_num <= 5; loop_num++) {
  //  // for (size_t loop_num = 1; loop_num <= 1; loop_num++) {
  //  struct Sprite *new_sprite = sprite_manager_add(&(game->sprite_manager), &(mana->api.api_common), "./assets/textures/pbr/rust/roughness.png");
  //  new_sprite->sprite_common.position = (vec3){.x = 0, .y = 15.0f, .z = 0};
  //  new_sprite->sprite_common.rotation = (quat){.data[0] = M_PI / 3.25f, .data[1] = 0, .data[2] = 0, .data[3] = 1.0f};
  //  //     new_sprite->sprite_common.position = (vec3){.x = (float)loop_num, .y = (float)loop_num, .z = (float)loop_num};
  //  // new_sprite->sprite_common.rotation = (quat){.data[0] = (float)loop_num / 3.0f, .data[1] = (float)loop_num / 3.0f, .data[2] = (float)loop_num / 3.0f, .data[3] = 1.0f};
  //}
  //
  // sprite_manager_remove(&(game->sprite_manager), &(mana->api.api_common), 1);
  // sprite_manager_remove(&(game->sprite_manager), &(mana->api.api_common), 3);

  // struct ModelStaticShader *model_static_shader, struct APICommon *api_common, uint32_t width, uint32_t height, uint_fast8_t supersample_scale, struct GBufferCommon *gbuffer_common, bool depth_test, const uint_fast32_t msaa_samples, uint_fast32_t descriptors
  //  sprite_shader_init(&(sprite_manager->sprite_manager_common.sprite_shader), api_common, width, height, supersample_scale, gbuffer_common, true, msaa_samples, descriptors);

  // model_static_shader_init(&(game->model_static_shader), &(mana->api.api_common), window->renderer.renderer_settings.width, window->renderer.renderer_settings.height, window->swap_chain->swap_chain_common.supersample_scale, &(window->gbuffer->gbuffer_common), true, window->renderer.renderer_settings.msaa_samples, 2048);

  // model_static_shader_init(&(game->model_static_shader), &(mana->api.api_common), &(window->swap_chain->swap_chain_common), &(window->gbuffer->gbuffer_common), true, window->renderer.renderer_settings.msaa_samples, 2048);
  // model_shader_init(&(game->model_shader), &(mana->api.api_common), &(window->swap_chain->swap_chain_common), &(window->gbuffer->gbuffer_common), true, window->renderer.renderer_settings.msaa_samples, 2048);

  // struct ModelSettings model1 = {"./assets/models/testmodel/model.dae", 3, &game->model_shader.shader,
  //                                texture_manager_get(&game->texture_manager, "./assets/textures/pbr/rust/diffuse.png"),
  //                                texture_manager_get(&game->texture_manager, "./assets/textures/pbr/rust/normal.png"),
  //                                texture_manager_get(&game->texture_manager, "./assets/textures/pbr/rust/metallic.png"),
  //                                texture_manager_get(&game->texture_manager, "./assets/textures/pbr/rust/roughness.png"),
  //                                texture_manager_get(&game->texture_manager, "./assets/textures/pbr/rust/ao.png")};
  //
  // struct ModelSettings model2 = {"./assets/models/swords/nusword.dae", 3, &game->model_static_shader.shader,
  //                               texture_manager_get(&(game->texture_manager), L"/textures/pbr/rust/albedo.png"),  // texture_cache_get(&game->texture_cache, "./assets/models/swords/colorgrid.png"),
  //                               texture_manager_get(&(game->texture_manager), L"/textures/pbr/rust/normal.png"),
  //                               texture_manager_get(&(game->texture_manager), L"/textures/pbr/rust/metallic.png"),
  //                               texture_manager_get(&(game->texture_manager), L"/textures/pbr/rust/roughness.png"),
  //                               texture_manager_get(&(game->texture_manager), L"/textures/pbr/rust/ao.png")};
  //
  // model_cache_init(&(game->model_cache));
  //// model_cache_add(&(game->model_cache), &(mana->api.api_common), &model1, 0);
  // model_cache_add(&(game->model_cache), &(mana->api.api_common), &model2, 1);
  //
  // array_list_init(&(game->models));
  //// array_list_add(&(game->models), model_cache_get(&(game->model_cache), &(mana->api.api_common), "./assets/models/testmodel/model.dae"));
  // array_list_add(&(game->models), model_cache_get(&(game->model_cache), &(mana->api.api_common), "./assets/models/swords/nusword.dae"));
}

void game_delete(struct Game* game, struct Mana* mana) {
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

  // game->mario_position.y -= 1.5f * delta_time;
  game->mario->sprite_common.position = game->mario_position;
  // game->mario->sprite_common.rotation = quaternion_rotate_by_vector((quat){.data[0] = 0, .data[1] = game->mario_rotation, .data[2] = 0, .data[3] = 1.0f}, (vec3){.x = 0, .y = 1, .z = 0});
  game->player.look_at_pos = (vec3d){.x = game->mario_position.x, .y = game->mario_position.y, .z = game->mario_position.z};
  // game->player.camera.look_at_azimuth = game->mario_drive_rotation;

  float move_speed = 30.0f;
  float rotation_speed = 1.5f - game->mario_speed / 50.0f;

  vec3 forward = {.x = cosf(game->player.camera.look_at_azimuth), .y = sinf(game->player.camera.look_at_azimuth), .z = 0.0f};

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

  game->mario_position.x += forward.x * game->mario_speed * delta_time;
  game->mario_position.y += forward.y * game->mario_speed * delta_time;

  game->mario_speed *= 0.999f * (1.0f - delta_time);

  for (int whispy_num = 0; whispy_num < 5; whispy_num++) {
    struct Sprite* whispy = game->whispy[whispy_num];

    // Make sprite face the camera (Y-axis billboard)
    float angle = game->player.camera.look_at_azimuth;
    mat4 rotation = mat4_rotate(MAT4_IDENTITY, -M_PI / 2, (vec3){.x = 0.5, .y = 0.0, .z = 0.0});
    rotation = mat4_rotate(rotation, -angle + M_PI / 2, (vec3){.x = 0.0f, .y = 1.0f, .z = 0.0f});
    whispy->sprite_common.rotation = mat4_to_quaternion(rotation);
  }

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
    //
    // for (size_t model_num = 0; model_num < array_list_size(&(game->models)); model_num++)
    //  model_update_uniforms(array_list_get(&(game->models), model_num), api_common, window->gbuffer, game->camera.fly_pos, vec3d_to_vec3(game->camera.fly_pos));

    // GBuffer, only texture that needs to be multisampled
    gbuffer_start(window->gbuffer, &(window->swap_chain->swap_chain_common), window->renderer.renderer_settings.msaa_samples);

    // Transparent sprites
    sprite_manager_render(&(game->sprite_manager), &(window->gbuffer->gbuffer_common));
    //
    // for (size_t model_num = 0; model_num < array_list_size(&(game->models)); model_num++)
    //  model_render(array_list_get(&(game->models), model_num), window->gbuffer, delta_time / 40.0);

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
