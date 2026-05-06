#pragma once

#include <mana/core/audio/audio.h>
#include <mana/core/graphics/entities/sprite/sprite.h>
#include <mana/core/graphics/entities/water/water.h>
#include <mana/core/graphics/shaders/modelshader.h>
#include <mana/core/graphics/shaders/modelstaticshader.h>
#include <mana/core/graphics/shaders/watershader.h>
#include <mana/core/graphics/utilities/camera.h>
#include <mana/core/graphics/utilities/modelcache.h>
#include <mana/core/graphics/utilities/spritemanager/spritemanager.h>
#include <mana/core/graphics/utilities/texturemanager/texturemanager.h>
#include <mana/core/utilities/xmlparser.h>
#include <mana/mana.h>

#include "drivingml/core/ac_model.h"
#include "drivingml/core/player.h"

#define DRIVE_OVERRIDE FALSE
#define DEPLOY_MODE TRUE

#define MAX_MARKERS 32
#define MAX_MARKER_ID 256
#define MAX_NEXT_MARKERS 2
#define MAX_NPCS 8
#define MAX_TREES 32

// Hyperparameter:
// 0.0 = prefer safest path
// 1.0 = prefer riskiest path
#define AI_RISK_PREFERENCE 0.0f

struct MarkerNext {
  i32 marker_id;
  i32 marker_index;
  r32 risk;
};

struct MarkerData {
  i32 id;
  struct Sprite* sprite;

  i32 next_count;
  struct MarkerNext next[MAX_NEXT_MARKERS];
};

struct GameMap {
  struct MarkerData markers[MAX_MARKERS];
  i32 marker_id_to_index[MAX_MARKER_ID];

  struct Sprite* trees[MAX_TREES];

  struct Model* track_model;
  struct Model* plane_model;

  vec3 start_pos;
  float start_heading;
};

struct NPC {
  struct ACModel model;
  r32 speed;
  vec3 position;
  r32 heading;
  r32 last_action[2];
  r32 prev_y;
  i32 current_marker;
  i32 last_marker;
  r32 risk_preference;
  struct Sprite* sprite;
};

struct Tentacle {
  struct Sprite* sprite;
  r32 accum;
  r32 accum_limit;
  u8 frame;
  u8 max_frames;
};

struct Game {
  struct Window* window;

  struct AudioEngine audio_engine;

  float audio_volome;

  r64 state_update_accum;

  struct AudioClip raw_music;
  struct AudioClipF32 music_sfx;

  struct AudioClip raw_fart_sfx;
  struct AudioClipF32 fart_sfx;

  struct Sprite* floor_plane;

  struct Sprite* boat1;

  struct Sprite* cloud1;
  struct Sprite* cloud2;

  struct Sprite* aero;

  struct Tentacle tentacle;
  struct GameMap game_map;

  struct Model* test_model;
  struct Model* test_static_model;
  struct Model* coin_model;

  SOCKET sock;

  struct ArrayList models;

  struct WaterShader water_shader;
  struct Water water;

  struct SpriteManager sprite_manager;
  struct TextureManager texture_manager;
  struct ModelCache model_cache;

  struct Player player;

  struct NPC npcs[MAX_NPCS];

  r32 previous_reward;

  i32 start_timer;
  i32 timer;

  i32 camera_current_follow_kart;

  i32 total_markers;
  i32 current_npcs;
  i32 total_trees;
};

void game_init(struct Game* game, struct Mana* mana, struct Window* window);
void game_delete(struct Game* game, struct Mana* mana);
void game_update(struct Game* game, struct Mana* mana, r64 delta_time);
void game_render(struct Game* game, struct Mana* mana, r64 delta_time);

internal r32 clamp01(r32 x) {
  if (x < 0.0f) return 0.0f;
  if (x > 1.0f) return 1.0f;
  return x;
}

internal i32 game_map_marker_index_from_id(struct GameMap* game_map, i32 marker_id) {
  if (marker_id < 0 || marker_id >= MAX_MARKER_ID)
    return -1;

  return game_map->marker_id_to_index[marker_id];
}

internal i32 choose_next_marker_by_risk(struct GameMap* game_map, i32 marker_index, r32 risk_preference) {
  if (marker_index < 0 || marker_index >= MAX_MARKERS)
    return -1;

  struct MarkerData* marker = &game_map->markers[marker_index];

  if (marker->next_count <= 0)
    return game_map_marker_index_from_id(game_map, 0);

  i32 best_next = 0;

  for (i32 i = 1; i < marker->next_count; i++) {
    r32 current_best_risk = marker->next[best_next].risk;
    r32 candidate_risk = marker->next[i].risk;

    if (risk_preference >= 0.5f) {
      // High-risk AI chooses the highest risk path.
      if (candidate_risk > current_best_risk)
        best_next = i;
    } else {
      // Low-risk AI chooses the lowest risk path.
      if (candidate_risk < current_best_risk)
        best_next = i;
    }
  }

  return marker->next[best_next].marker_index;
}

struct TargetOption {
  i32 marker_index;
  r32 risk;
};

internal void get_current_target_options(struct GameMap* game_map, struct NPC* npc, struct TargetOption out_options[2]) {
  out_options[0].marker_index = npc->current_marker;
  out_options[0].risk = 0.0f;
  out_options[1].marker_index = npc->current_marker;
  out_options[1].risk = 0.0f;

  if (npc->last_marker < 0)
    return;

  struct MarkerData* last_marker = &game_map->markers[npc->last_marker];

  if (last_marker->next_count <= 0)
    return;

  for (i32 i = 0; i < last_marker->next_count && i < 2; i++) {
    out_options[i].marker_index = last_marker->next[i].marker_index;
    out_options[i].risk = last_marker->next[i].risk;
  }

  // If there is only one option, duplicate it so the model always receives two.
  if (last_marker->next_count == 1) {
    out_options[1] = out_options[0];
  }
}

internal void marker_option_to_local_features(
    struct GameMap* game_map,
    struct NPC* npc,
    struct TargetOption option,
    r32 norm,
    r32* out_forward,
    r32* out_right,
    r32* out_risk) {
  if (option.marker_index < 0 || option.marker_index >= MAX_MARKERS) {
    *out_forward = 0.0f;
    *out_right = 0.0f;
    *out_risk = 0.0f;
    return;
  }

  vec3 target_pos = game_map->markers[option.marker_index].sprite->sprite_common.position;

  r32 dxw = target_pos.x - npc->position.x;
  r32 dzw = target_pos.z - npc->position.z;

  r32 fx = -real32_cos(npc->heading);
  r32 fz = real32_sin(npc->heading);
  r32 rx = -real32_sin(npc->heading);
  r32 rz = -real32_cos(npc->heading);

  r32 forward_err = dxw * fx + dzw * fz;
  r32 right_err = dxw * rx + dzw * rz;

  *out_forward = forward_err / norm;
  *out_right = right_err / norm;
  *out_risk = clamp01(option.risk);
}

internal const char* track_surface_type_name(i32 surface_type) {
  switch (surface_type) {
    case TRACK_SURFACE_GRASS:
      return "grass";
    case TRACK_SURFACE_ROAD:
      return "road";
    case TRACK_SURFACE_SAND_DRIVE:
      return "sand_drive";
    case TRACK_SURFACE_SAND:
      return "sand";
    case TRACK_SURFACE_WALL:
      return "wall";
    case TRACK_SURFACE_OOB:
      return "out_of_bounds";
    default:
      return "unknown";
  }
}

internal b8 point_in_triangle_xz(r32 px, r32 pz, vec3 a, vec3 b, vec3 c, r32* out_u, r32* out_v, r32* out_w) {
  r32 v0x = b.x - a.x;
  r32 v0z = b.z - a.z;
  r32 v1x = c.x - a.x;
  r32 v1z = c.z - a.z;
  r32 v2x = px - a.x;
  r32 v2z = pz - a.z;

  r32 d00 = v0x * v0x + v0z * v0z;
  r32 d01 = v0x * v1x + v0z * v1z;
  r32 d11 = v1x * v1x + v1z * v1z;
  r32 d20 = v2x * v0x + v2z * v0z;
  r32 d21 = v2x * v1x + v2z * v1z;

  r32 denom = d00 * d11 - d01 * d01;

  if (real32_fabs(denom) < 0.00001f)
    return FALSE;

  r32 v = (d11 * d20 - d01 * d21) / denom;
  r32 w = (d00 * d21 - d01 * d20) / denom;
  r32 u = 1.0f - v - w;

  const r32 eps = -0.0001f;

  if (u >= eps && v >= eps && w >= eps) {
    *out_u = u;
    *out_v = v;
    *out_w = w;
    return TRUE;
  }

  return FALSE;
}

internal vec3 track_local_to_world(struct Model* track_model, vec3 p) {
  vec3 pos = track_model->model_common.position;
  vec3 scale = track_model->model_common.scale;

  return (vec3){
      .x = pos.x + p.x * scale.x,
      .y = pos.y + p.y * scale.y,
      .z = pos.z + p.z * scale.z};
}

internal b8 track_get_height_at(struct Model* track_model, r32 world_x, r32 world_z, r32* out_y, i32* out_surface_type) {
  if (track_model == NULL || out_y == NULL)
    return FALSE;

  struct Mesh* mesh = track_model->model_common.model_mesh;
  if (mesh == NULL)
    return FALSE;

  if (mesh->mesh_common.indices == NULL || mesh->mesh_common.vertices == NULL)
    return FALSE;

  vec3 track_pos = track_model->model_common.position;
  vec3 track_scale = track_model->model_common.scale;

  if (track_scale.x == 0.0f || track_scale.y == 0.0f || track_scale.z == 0.0f)
    return FALSE;

  // Convert world kart position into track-local space.
  // This assumes the track has position + scale, but no rotation.
  r32 local_x = (world_x - track_pos.x) / track_scale.x;
  r32 local_z = (world_z - track_pos.z) / track_scale.z;

  size_t index_count = vector_size(mesh->mesh_common.indices);
  size_t vertex_count = vector_size(mesh->mesh_common.vertices);

  for (size_t i = 0; i + 2 < index_count; i += 3) {
    i32 ia = *(i32*)vector_get(mesh->mesh_common.indices, i + 0);
    i32 ib = *(i32*)vector_get(mesh->mesh_common.indices, i + 1);
    i32 ic = *(i32*)vector_get(mesh->mesh_common.indices, i + 2);

    if (ia < 0 || ib < 0 || ic < 0)
      continue;

    if ((size_t)ia >= vertex_count || (size_t)ib >= vertex_count || (size_t)ic >= vertex_count)
      continue;

    struct VertexModelStatic* va = (struct VertexModelStatic*)vector_get(mesh->mesh_common.vertices, (size_t)ia);
    struct VertexModelStatic* vb = (struct VertexModelStatic*)vector_get(mesh->mesh_common.vertices, (size_t)ib);
    struct VertexModelStatic* vc = (struct VertexModelStatic*)vector_get(mesh->mesh_common.vertices, (size_t)ic);

    if (va == NULL || vb == NULL || vc == NULL)
      continue;

    vec3 a = va->position;
    vec3 b = vb->position;
    vec3 c = vc->position;

    r32 u, v, w;

    // IMPORTANT: use local_x/local_z here, not world_x/world_z.
    if (point_in_triangle_xz(local_x, local_z, a, b, c, &u, &v, &w)) {
      r32 local_y = a.y * u + b.y * v + c.y * w;
      *out_y = track_pos.y + local_y * track_scale.y;

      if (out_surface_type != NULL) {
        size_t triangle_index = i / 3;

        if (mesh->mesh_common.triangle_surface_types != NULL &&
            triangle_index < vector_size(mesh->mesh_common.triangle_surface_types)) {
          *out_surface_type = *(i32*)vector_get(mesh->mesh_common.triangle_surface_types, triangle_index);
        } else {
          *out_surface_type = TRACK_SURFACE_UNKNOWN;
        }
      }

      return TRUE;
    }
  }

  return FALSE;
}

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

internal void load_map_from_xml(struct Game* game, struct Mana* mana, struct GameMap* game_map, const char* xml_path, const char* map_name) {
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
    game_map->track_model = model_cache_get(&(game->model_cache), &(mana->api.api_common), tex);

    r32 scale = sx ? (r32)atof(sx) : 25.0f;
    r32 x = px ? (r32)atof(px) : 0.0f;
    r32 y = py ? (r32)atof(py) : 0.0f;

    game_map->track_model->model_common.position = (vec3){.x = x, .y = 0.0f, .z = y};
    game_map->track_model->model_common.scale = (vec3){.x = scale, .y = scale, .z = scale};
  }

  game_map->start_pos = (vec3){.x = 0.0f, .y = 0.0f, .z = 0.0f};
  game_map->start_heading = 0.0f;

  struct XmlNode* start_node = xml_node_get_child(map_node, "start");

  if (start_node) {
    char* start_x_str = xml_node_get_attribute(start_node, "x");
    char* start_y_str = xml_node_get_attribute(start_node, "y");
    char* start_z_str = xml_node_get_attribute(start_node, "z");
    char* heading_str = xml_node_get_attribute(start_node, "heading");

    game_map->start_pos = (vec3){.x = start_x_str ? (r32)atof(start_x_str) : 0.0f, .y = start_y_str ? (r32)atof(start_y_str) : 0.0f, .z = start_z_str ? (r32)atof(start_z_str) : 0.0f};

    game_map->start_heading = heading_str ? (r32)atof(heading_str) : 0.0f;
  }

  struct XmlNode* markers_node = xml_node_get_child(map_node, "markers");

  for (i32 i = 0; i < MAX_MARKER_ID; i++)
    game_map->marker_id_to_index[i] = -1;

  for (i32 i = 0; i < MAX_MARKERS; i++) {
    game_map->markers[i].id = -1;
    game_map->markers[i].sprite = NULL;
    game_map->markers[i].next_count = 0;
  }

  if (markers_node) {
    struct ArrayList* marker_list = xml_node_get_children(markers_node, "marker");

    if (marker_list) {
      size_t raw_count = array_list_size(marker_list);
      size_t count = raw_count;

      if (count > MAX_MARKERS)
        count = MAX_MARKERS;

      game->total_markers = (i32)count;

      // First pass: load marker positions and IDs.
      for (size_t i = 0; i < count; i++) {
        struct XmlNode* marker = (struct XmlNode*)array_list_get(marker_list, i);

        char* id_str = xml_node_get_attribute(marker, "id");
        char* x_str = xml_node_get_attribute(marker, "x");
        char* y_str = xml_node_get_attribute(marker, "y");

        if (!x_str || !y_str)
          continue;

        i32 marker_id = id_str ? atoi(id_str) : (i32)i;

        if (marker_id < 0 || marker_id >= MAX_MARKER_ID) {
          printf("Marker id %d is out of range. Increase MAX_MARKER_ID.\n", marker_id);
          continue;
        }

        r32 x = (r32)atof(x_str);
        r32 y = (r32)atof(y_str);

        game_map->markers[i].id = marker_id;
        game_map->markers[i].next_count = 0;
        game_map->markers[i].sprite = sprite_manager_add_sprite(
            &(game->sprite_manager),
            &(mana->api.api_common),
            "/textures/marker.png");

        game_map->marker_id_to_index[marker_id] = (i32)i;

        place_marker(game_map->markers[i].sprite, x, y);
      }

      // Second pass: load <next> children now that all marker IDs are known.
      for (size_t i = 0; i < count; i++) {
        struct XmlNode* marker = (struct XmlNode*)array_list_get(marker_list, i);
        struct MarkerData* marker_data = &game_map->markers[i];

        struct ArrayList* next_list = xml_node_get_children(marker, "next");

        if (!next_list)
          continue;

        size_t next_count = array_list_size(next_list);

        for (size_t j = 0; j < next_count && j < MAX_NEXT_MARKERS; j++) {
          struct XmlNode* next = (struct XmlNode*)array_list_get(next_list, j);

          char* next_id_str = xml_node_get_attribute(next, "id");
          char* risk_str = xml_node_get_attribute(next, "risk");

          if (!next_id_str)
            continue;

          i32 next_id = atoi(next_id_str);
          i32 next_index = game_map_marker_index_from_id(game_map, next_id);

          if (next_index < 0) {
            printf("Marker %d has invalid next id %d\n", marker_data->id, next_id);
            continue;
          }

          r32 risk = risk_str ? (r32)atof(risk_str) : 0.0f;
          risk = clamp01(risk);

          i32 slot = marker_data->next_count;

          marker_data->next[slot].marker_id = next_id;
          marker_data->next[slot].marker_index = next_index;
          marker_data->next[slot].risk = risk;

          marker_data->next_count++;
        }
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

        game_map->trees[i] = sprite_manager_add_sprite(&(game->sprite_manager), &(mana->api.api_common), "/textures/tree.png");

        game_map->trees[i]->sprite_common.position = (vec3){.x = x, .y = 4.5f, .z = y};
        game_map->trees[i]->sprite_common.scale = (vec3){.x = 5.0f, .y = 5.0f, .z = 0.0f};

        mat4 rot = mat4_rotate(MAT4_IDENTITY, -(r32)R32_PI / 2, (vec3){.x = 0.5, .y = 0, .z = 0});
        game_map->trees[i]->sprite_common.rotation = mat4_to_quaternion(rot);
      }
    }
  }

  xml_parser_delete(root);
}
