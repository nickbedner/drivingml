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

#define EVAL_MODE TRUE

#define MAX_MARKERS 32
#define MAX_NPCS 4
#define MAX_TREES 32

struct NPC {
  struct ACModel model;
  r32 speed;
  vec3 position;
  r32 heading;
  r32 last_action[2];
  r32 prev_y;
  i32 current_marker;
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

  struct Sprite* floor_plane;
  struct Sprite* flag1;
  struct Sprite* flag2;

  struct Sprite* marker[MAX_MARKERS];
  struct Sprite* trees[MAX_TREES];

  struct Sprite* boat1;

  struct Sprite* cloud1;
  struct Sprite* cloud2;

  struct Sprite* aero;

  struct Tentacle tentacle;

  struct Model* track_model;
  struct Model* plane_model;
  struct Model* test_model;
  struct Model* test_static_model;
  struct Model* coin_model;

  struct AudioClip fart;

  SOCKET sock;

  struct ArrayList models;

  struct WaterShader water_shader;
  struct Water water;

  struct SpriteManager sprite_manager;
  struct TextureManager texture_manager;
  struct ModelCache model_cache;

  struct Player player;

  struct NPC npcs[MAX_NPCS];

  vec3 starting_pos;

  r32 previous_reward;
  r32 starting_heading;

  int start_timer;
  int timer;

  int camera_current_follow_kart;

  int total_markers;
  int current_npcs;
  int total_trees;
};

void game_init(struct Game* game, struct Mana* mana, struct Window* window);
void game_delete(struct Game* game, struct Mana* mana);
void game_update(struct Game* game, struct Mana* mana, r64 delta_time);
void game_render(struct Game* game, struct Mana* mana, r64 delta_time);
