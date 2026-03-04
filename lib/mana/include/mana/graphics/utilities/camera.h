#pragma once

#include "mana/core/corecommon.h"
#include "mana/graphics/graphicscommon.h"
#include "mana/graphics/render/window.h"
#include "mana/math/advmath.h"

// TODO: Add attached camera to hook onto entities like the player, other player, second person view, etc.
enum CAMERA_STATE {
  CAMERA_FLY = 0,
  CAMERA_LOOK_AT
};

// Switch to double precision
struct Camera {
  enum CAMERA_STATE camera_state;
  // Render
  vec3d eye;
  vec3d target;
  vec3d up;
  double zoom;

  int is_interpoliating;
  vec3d lerp_eye;
  vec3d lerp_target;
  vec3d lerp_up;
  double lerp_zoom;

  double field_of_view_y;
  double aspect_ratio;

  // Fly
  vec3d fly_pos;
  vec3d fly_eye;
  vec3d fly_target;
  vec3d fly_up;
  vec3d fly_look;
  vec3d fly_right;
  double fly_zoom;
  double fly_movement_rate;

  // Look at
  vec3d look_at_eye;
  vec3d look_at_target;
  vec3d look_at_up;

  double look_at_zoom_factor;
  double look_at_zoom_rate_range_adjustment;
  double look_at_maximum_zoom_rate;
  double look_at_minimum_zoom_rate;

  double look_at_rotate_factor;
  double look_at_rotate_rate_range_adjustment;
  double look_at_minimum_rotate_rate;
  double look_at_maximum_rotate_rate;

  double look_at_azimuth;
  double look_at_elevation;
  double look_at_range;

  vec3d look_at_center_point;
  mat3d look_at_fixed_to_local_rotation;
};

static inline void camera_init(struct Camera* camera, double max_radius) {
  camera->camera_state = CAMERA_FLY;

  camera->eye = (vec3d){.x = 0.0, .y = -1.0, .z = 0.0};
  camera->target = VEC3D_ZERO;
  camera->up = (vec3d){.x = 0.0, .y = 0.0, .z = 1.0};
  camera->zoom = 45.0;

  camera->is_interpoliating = 0;
  camera->lerp_eye = (vec3d){.x = 0.0, .y = -1.0, .z = 0.0};
  camera->lerp_target = VEC3D_ZERO;
  camera->lerp_up = (vec3d){.x = 0.0, .y = 0.0, .z = 1.0};
  camera->lerp_zoom = 45.0;

  camera->field_of_view_y = M_PI / 6.0;
  camera->aspect_ratio = 1.0;

  // Fly
  camera->fly_pos = VEC3D_ZERO;
  camera->fly_eye = camera->eye;
  camera->fly_target = camera->target;
  camera->fly_up = camera->up;
  camera->fly_zoom = camera->zoom;
  camera->fly_look = VEC3D_ZERO;
  camera->fly_right = VEC3D_ZERO;
  camera->fly_movement_rate = 10.0;

  // Look at
  camera->look_at_eye = camera->eye;
  camera->look_at_target = camera->target;
  camera->look_at_up = camera->up;

  camera->look_at_center_point = camera->target;
  camera->look_at_fixed_to_local_rotation = MAT3D_IDENTITY;
  camera->look_at_zoom_factor = 5.0;
  camera->look_at_zoom_rate_range_adjustment = max_radius;
  camera->look_at_maximum_zoom_rate = DBL_MAX;
  camera->look_at_minimum_zoom_rate = max_radius / 100.0;
  camera->look_at_rotate_factor = 1.0 / max_radius;
  camera->look_at_rotate_rate_range_adjustment = max_radius;
  camera->look_at_maximum_rotate_rate = 1.0;
  camera->look_at_minimum_rotate_rate = 1.0 / 5000.0;
  camera->look_at_range = max_radius * 2.0;
}

// TODO: Just make this a quick animation instead of hacking it together for no reason
// static inline void camera_lerp_end(struct Camera* camera, vec3d lerp_eye, vec3d lerp_target, vec3d lerp_up) {
//   camera->is_interpoliating = 1;
//   camera->lerp_eye = lerp_eye;
//   camera->lerp_target = lerp_target;
//   camera->lerp_up = lerp_up;
// }

static inline void camera_update(struct Camera* camera) {
  // if (camera->is_interpoliating == 1) {
  //   // if (fabs(camera->zoom - camera->lerp_zoom) < 0.1)
  //   //   camera->is_interpoliating = 0;
  //
  //  double interp_speed = 0.1;
  //  camera->eye = vec3d_add(camera->eye, vec3d_scale(vec3d_sub(camera->lerp_eye, camera->eye), interp_speed));
  //  camera->target = vec3d_add(camera->target, vec3d_scale(vec3d_sub(camera->lerp_target, camera->target), interp_speed));
  //  camera->up = vec3d_add(camera->up, vec3d_scale(vec3d_sub(camera->lerp_up, camera->up), interp_speed));
  //  camera->zoom = camera->zoom + ((camera->lerp_zoom - camera->zoom) * interp_speed);
  //} else {
  if (camera->camera_state == CAMERA_FLY) {
    camera->eye = camera->fly_eye;
    camera->target = camera->fly_target;
    camera->up = camera->fly_up;
    camera->zoom = camera->fly_zoom;
  } else if (camera->camera_state == CAMERA_LOOK_AT) {
    camera->eye = camera->look_at_eye;
    camera->target = camera->look_at_target;
    camera->up = camera->look_at_up;
    camera->zoom = camera->look_at_zoom_factor;
  }
  //}
}

static inline vec3d camera_get_pos(struct Camera* camera) {
  return camera->eye;
}

static inline mat4 camera_get_projection_matrix(struct Camera* camera, struct Window* window) {
  float f = 1.0f / (float)tan(degree_to_radian_d(camera->zoom) / 2.0);
  mat4 dest = MAT4_ZERO;
  dest.vecs[0].data[0] = f / ((float)window->renderer.renderer_settings.width / (float)window->renderer.renderer_settings.height);
  dest.vecs[1].data[1] = f;
  dest.vecs[2].data[3] = -1.0f;
  dest.vecs[3].data[2] = Z_NEAR;

  return dest;
}

static inline mat4 camera_get_view_matrix(struct Camera* camera) {
  return mat4d_to_mat4(mat4d_look_at(camera->eye, camera->target, camera->up));
}

static inline vec3d camera_forward(struct Camera* camera) {
  return vec3d_normalise(vec3d_sub(camera->fly_target, camera->fly_eye));
}

static inline vec3d camera_right(struct Camera* camera) {
  return vec3d_normalise(vec3d_cross_product(camera_forward(camera), camera->fly_up));
}

static inline void camera_fly_move_forward(struct Camera* camera, double seconds) {
  camera->fly_pos = vec3d_add(camera->fly_pos, vec3d_scale(camera_forward(camera), camera->fly_movement_rate * seconds));
}

static inline void camera_fly_move_backward(struct Camera* camera, double seconds) {
  camera->fly_pos = vec3d_sub(camera->fly_pos, vec3d_scale(camera_forward(camera), camera->fly_movement_rate * seconds));
}

static inline void camera_fly_move_left(struct Camera* camera, double seconds) {
  camera->fly_pos = vec3d_sub(camera->fly_pos, vec3d_scale(vec3d_cross_product(camera_forward(camera), camera->fly_up), camera->fly_movement_rate * seconds));
}

static inline void camera_fly_move_right(struct Camera* camera, double seconds) {
  camera->fly_pos = vec3d_add(camera->fly_pos, vec3d_scale(vec3d_cross_product(camera_forward(camera), camera->fly_up), camera->fly_movement_rate * seconds));
}

static inline void camera_fly_move_up(struct Camera* camera, double seconds) {
  camera->fly_pos = vec3d_add(camera->fly_pos, vec3d_scale(camera->fly_up, camera->fly_movement_rate * seconds));
}

static inline void camera_fly_move_down(struct Camera* camera, double seconds) {
  camera->fly_pos = vec3d_sub(camera->fly_pos, vec3d_scale(camera->fly_up, camera->fly_movement_rate * seconds));
}

static inline void camera_fly_roll_left(struct Camera* camera, double seconds) {
  camera->fly_up = vec3d_transform(camera->fly_up, quaterniond_create_from_axix_angle(camera->fly_look, (camera->fly_movement_rate / 25.0) * seconds));
  camera->fly_right = vec3d_cross_product(camera->fly_look, camera->fly_up);

  camera->fly_up = vec3d_normalise(camera->fly_up);
  camera->fly_right = vec3d_normalise(camera->fly_right);
}

static inline void camera_fly_roll_right(struct Camera* camera, double seconds) {
  camera->fly_up = vec3d_transform(camera->fly_up, quaterniond_create_from_axix_angle(camera->fly_look, (camera->fly_movement_rate / 25.0) * -seconds));
  camera->fly_right = vec3d_cross_product(camera->fly_look, camera->fly_up);

  camera->fly_up = vec3d_normalise(camera->fly_up);
  camera->fly_right = vec3d_normalise(camera->fly_right);
}

static inline double camera_field_of_view_x(struct Camera* camera) {
  return (2.0 * atan(camera->aspect_ratio * tan(camera->field_of_view_y * 0.5)));
}

static inline void camera_zoom_to_target(struct Camera* camera, double radius) {
  vec3d to_eye = vec3d_normalise(vec3d_sub(camera->look_at_eye, camera->look_at_target));

  double sin_val = sin(MIN(camera_field_of_view_x(camera), camera->field_of_view_y) * 0.5);
  double distance = radius / sin_val;
  camera->look_at_eye = vec3d_add(camera->look_at_target, vec3d_scale(to_eye, distance));
}

static inline void camera_rotate(struct Camera* camera, int width, int height, int window_width, int window_height) {
  if (camera->camera_state == CAMERA_FLY) {
    double horizontal_window_ratio = (double)width / (double)window_width;
    double vertical_window_ratio = (double)height / (double)window_height;

    quatd horizontal_rotation = quaterniond_create_from_axix_angle(camera->fly_up, -horizontal_window_ratio);
    quatd vertical_rotation = quaterniond_create_from_axix_angle(camera->fly_right, -vertical_window_ratio);  // Note: Vulkan inverted y

    camera->fly_look = vec3d_transform(camera->fly_look, horizontal_rotation);
    camera->fly_look = vec3d_transform(camera->fly_look, vertical_rotation);
    camera->fly_up = vec3d_transform(camera->fly_up, horizontal_rotation);
    camera->fly_up = vec3d_transform(camera->fly_up, vertical_rotation);
    camera->fly_right = vec3d_cross_product(camera->fly_look, camera->fly_up);

    camera->fly_look = vec3d_normalise(camera->fly_look);
    camera->fly_up = vec3d_normalise(camera->fly_up);
    camera->fly_right = vec3d_normalise(camera->fly_right);
  } else if (camera->camera_state == CAMERA_LOOK_AT) {
    double rotate_rate = camera->look_at_rotate_factor * (camera->look_at_range - camera->look_at_rotate_rate_range_adjustment);
    if (rotate_rate > camera->look_at_maximum_rotate_rate)
      rotate_rate = camera->look_at_maximum_rotate_rate;
    if (rotate_rate < camera->look_at_minimum_rotate_rate)
      rotate_rate = camera->look_at_minimum_rotate_rate;

    double azimuth_window_ratio = (double)width / (double)window_width;
    double elevation_window_ratio = (double)height / (double)window_height;

    camera->look_at_azimuth -= rotate_rate * azimuth_window_ratio * (2.0 * M_PI);
    camera->look_at_elevation += rotate_rate * elevation_window_ratio * M_PI;

    while (camera->look_at_azimuth > M_PI)
      camera->look_at_azimuth -= (2.0 * M_PI);
    while (camera->look_at_azimuth < -M_PI)
      camera->look_at_azimuth += (2.0 * M_PI);

    while (camera->look_at_elevation < -M_PI_2)
      camera->look_at_elevation = -M_PI_2;
    while (camera->look_at_elevation > M_PI_2)
      camera->look_at_elevation = M_PI_2;
  }
}

static inline void camera_update_parameters_from_camera(struct Camera* camera) {
  // Fly
  camera->fly_pos = camera->fly_eye;
  camera->fly_look = camera_forward(camera);
  camera->fly_right = camera_right(camera);

  // Look at
  vec3d eye_position = mat3d_transform_transpose(camera->look_at_fixed_to_local_rotation, vec3d_sub(camera->look_at_eye, camera->look_at_target));
  vec3d up = mat3d_transform_transpose(camera->look_at_fixed_to_local_rotation, camera->look_at_up);

  camera->look_at_range = sqrt(eye_position.x * eye_position.x + eye_position.y * eye_position.y + eye_position.z * eye_position.z);
  camera->look_at_elevation = asin(eye_position.z / camera->look_at_range);

  if ((eye_position.x * eye_position.x + eye_position.y * eye_position.y) < (up.x * up.x + up.y * up.y)) {
    if (eye_position.z > 0.0)
      camera->look_at_azimuth = atan2(-up.y, -up.x);
    else
      camera->look_at_azimuth = atan2(up.y, up.x);
  } else
    camera->look_at_azimuth = atan2(eye_position.y, eye_position.x);
}

static inline void camera_update_camera_from_parameters(struct Camera* camera) {
  // Fly
  camera->fly_eye = camera->fly_pos;
  camera->fly_target = vec3d_add(camera->fly_pos, camera->fly_look);

  // Look at
  camera->look_at_target = camera->look_at_center_point;

  double range_time_sin_elevation = camera->look_at_range * cos(camera->look_at_elevation);
  camera->look_at_eye = (vec3d){.x = range_time_sin_elevation * cos(camera->look_at_azimuth), .y = range_time_sin_elevation * sin(camera->look_at_azimuth), .z = camera->look_at_range * sin(camera->look_at_elevation)};

  vec3d right = vec3d_cross_product(camera->look_at_eye, (vec3d){.x = 0.0, .y = 0.0, .z = 1.0});
  camera->look_at_up = vec3d_normalise(vec3d_cross_product(right, camera->look_at_eye));

  if (isnan(camera->look_at_up.x))
    camera->look_at_up = (vec3d){.x = -cos(camera->look_at_azimuth), .y = -sin(camera->look_at_azimuth), .z = 0.0};

  mat3d local_to_fixed = mat3d_transpose(camera->look_at_fixed_to_local_rotation);
  camera->look_at_eye = mat3d_transform_transpose(local_to_fixed, camera->look_at_eye);
  camera->look_at_eye = vec3d_add(camera->look_at_eye, camera->look_at_center_point);
  camera->look_at_up = mat3d_transform_transpose(local_to_fixed, camera->look_at_up);
}

static inline void camera_look_at_view_point(struct Camera* camera, float longitude, float latitude, vec3d center_point) {
  camera->look_at_center_point = center_point;

  double cos_lon = cos((double)longitude);
  double cos_lat = cos((double)latitude);
  double sin_lon = sin((double)longitude);
  double sin_lat = sin((double)latitude);
  camera->look_at_fixed_to_local_rotation = (mat3d){.m00 = -sin_lon, .m01 = cos_lon, .m02 = 0.0, .m10 = -sin_lat * cos_lon, .m11 = -sin_lat * sin_lon, .m12 = cos_lat, .m20 = cos_lat * cos_lon, .m21 = cos_lat * sin_lon, .m22 = sin_lat};

  camera_update_camera_from_parameters(camera);
}

static inline void camera_look_at_zoom(struct Camera* camera, double amount, uint_fast32_t window_height) {
  if (camera->camera_state == CAMERA_LOOK_AT) {
    double zoom_rate = camera->look_at_zoom_factor * (camera->look_at_range - camera->look_at_zoom_rate_range_adjustment);
    if (zoom_rate > camera->look_at_maximum_zoom_rate)
      zoom_rate = camera->look_at_maximum_zoom_rate;
    if (zoom_rate < camera->look_at_minimum_zoom_rate)
      zoom_rate = camera->look_at_minimum_zoom_rate;

    double range_window_ratio = amount / (double)window_height;
    camera->look_at_range -= zoom_rate * range_window_ratio;
  }
}
