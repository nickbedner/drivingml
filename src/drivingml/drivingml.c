#include "drivingml/drivingml.h"

uint_fast8_t drivingml_init(struct DrivingML* drivingml) {
  struct RendererSettings main_window_renderer_settings = {0};
  main_window_renderer_settings.width = 1280;
  main_window_renderer_settings.height = 720;
  main_window_renderer_settings.supersample_scale = 1;
  main_window_renderer_settings.msaa_samples = 1;
  main_window_renderer_settings.vsync = true;
  main_window_renderer_settings.preferred_api_type = API_VULKAN;

  // TODO: Load preferred from file
  const uint_fast8_t mana_init_error = mana_init(&(drivingml->mana), main_window_renderer_settings.preferred_api_type);

  switch (mana_init_error) {
    case (MANA_SUCCESS): {
      break;
    }
    case (MANA_GPU_API_ERROR): {
      log_message(LOG_SEVERITY_ERROR, "Failed to setup Mana!\n");
      return DRIVINGML_MANA_ERROR;
    }
    default: {
      log_message(LOG_SEVERITY_CRITICAL, "Unknown Mana error! Error code: %d\n", mana_init_error);
      return DRIVINGML_MANA_ERROR;
    }
  }

  const uint_fast8_t window_error = window_init(&(drivingml->window), &(drivingml->mana.api.api_common), PROJECT_NAME, &(main_window_renderer_settings));

  switch (window_error) {
    case (WINDOW_SUCCESS): {
      break;
    }
    case (WINDOW_ERROR): {
      log_message(LOG_SEVERITY_ERROR, "Error creating window!\n");
      return DRIVINGML_WINDOW_ERROR;
    }
    default: {
      log_message(LOG_SEVERITY_CRITICAL, "Unknown window error! Error code: %d\n", window_error);
      return DRIVINGML_MANA_ERROR;
    }
  }

  game_init(&(drivingml->game), &(drivingml->mana), &(drivingml->window));

  return DRIVINGML_SUCCESS;
}

void drivingml_delete(struct DrivingML* drivingml) {
  game_delete(&(drivingml->game), &(drivingml->mana));
  window_delete(&(drivingml->window));
  mana_delete(&(drivingml->mana));
}

void drivingml_start(struct DrivingML* drivingml) {
  const double sim_dt = 1.0 / 60.0;
  double previous_time = get_high_resolution_time();
  double accumulator = 0.0;

  while (!drivingml->window.should_close) {
    double current_time = get_high_resolution_time();
    double frame_time = current_time - previous_time;

    previous_time = current_time;

    if (frame_time > 0.25) frame_time = 0.25;  // avoid giant catch-up
    accumulator += frame_time;

    window_prepare_frame(&(drivingml->window));

    int max_steps = 5;
    int steps = 0;

#ifndef EVAL_MODE
    // In training mode we want to run as many sim steps as possible to speed up training, but still render every frame so we can see what's going on
    while (accumulator >= sim_dt && steps < max_steps) {
      game_update(&(drivingml->game), &(drivingml->mana), sim_dt);
      accumulator -= sim_dt;
      steps++;
    }
#else
    game_update(&(drivingml->game), &(drivingml->mana), 0.01666666666);
#endif
    // render once per displayed frame
    game_render(&(drivingml->game), &(drivingml->mana), frame_time);
    window_end_frame(&(drivingml->window));
  }
}