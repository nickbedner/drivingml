#include "drivingml/drivingml.h"

uint_fast8_t drivingml_init(struct DrivingML* drivingml) {
  struct RendererSettings main_window_renderer_settings = {0};
  main_window_renderer_settings.width = 1280;
  main_window_renderer_settings.height = 720;
  main_window_renderer_settings.supersample_scale = 1;
  main_window_renderer_settings.msaa_samples = 2;
  main_window_renderer_settings.vsync = false;
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
  // TODO: Add a way to change the target frame rate
  const int target_frame_rate = 999;
  const double target_frame_time = 1.0 / (double)target_frame_rate;

  double prev_time = get_high_resolution_time();
  double fps_timer = prev_time;
  int frame_count = 0;

  while (!drivingml->window.should_close) {
    double current_time = get_high_resolution_time();
    double delta_time = current_time - prev_time;
    prev_time = current_time;

    // Frame update
    window_prepare_frame(&(drivingml->window));
    // game_update(&(drivingml->game), &(drivingml->mana), delta_time);
    game_update(&(drivingml->game), &(drivingml->mana), 0.01666666666);
    window_end_frame(&(drivingml->window));

    frame_count++;

    // FPS logging once per second
    if (current_time - fps_timer >= 1.0) {
      double fps = frame_count / (current_time - fps_timer);
      printf("FPS: %.2f\n", fps);
      fps_timer = current_time;
      frame_count = 0;
    }

    double ms_per_frame = delta_time * 1000.0;
    // printf("Frame Time: %.3f ms\n", ms_per_frame);

    // Frame limiting (if vsync is off)
    if (!drivingml->window.vsync) {
      double frame_elapsed = get_high_resolution_time() - current_time;
      double remaining_time = target_frame_time - frame_elapsed;

      if (remaining_time > 0.001) {  // avoid sleeping for too little
        Sleep((DWORD)(remaining_time * 1000.0));
      }
    }
  }
}
