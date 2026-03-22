#pragma once

#include <float.h>
#include <mana/graphics/entities/sprite/sprite.h>
#include <mana/graphics/entities/water/water.h>
#include <mana/graphics/shaders/modelshader.h>
#include <mana/graphics/shaders/modelstaticshader.h>
#include <mana/graphics/shaders/watershader.h>
#include <mana/graphics/utilities/camera.h>
#include <mana/graphics/utilities/modelcache.h>
#include <mana/graphics/utilities/spritemanager/spritemanager.h>
#include <mana/graphics/utilities/texturemanager/texturemanager.h>
#include <mana/mana.h>
#include <mana/utilities/xmlparser.h>

#include "drivingml/core/ac_model.h"
#include "drivingml/core/player.h"

#define EVAL_MODE true

#define MAX_MARKERS 32
#define MAX_NPCS 4
#define MAX_TREES 32

struct NPC {
  struct ACModel model;
  float speed;
  vec3 position;
  float heading;
  float last_action[2];
  float prev_y;
  int current_marker;
  struct Sprite* sprite;
};

struct Game {
  struct Window* window;

  struct ArrayList models;

  struct ModelShader model_shader;
  struct ModelStaticShader model_static_shader;

  struct WaterShader water_shader;
  struct Water water;

  struct SpriteManager sprite_manager;
  struct TextureManager texture_manager;
  struct ModelCache model_cache;

  struct Player player;

  SOCKET sock;
  float previous_reward;

  int start_timer;
  int timer;

  // ACModel model;
  // float mario_speed;
  // float car_heading;
  // vec3 mario_position;
  // struct Sprite* mario;
  struct Sprite* track;
  // struct Sprite* start;
  // struct Sprite* finish;
  // struct Sprite* fence;
  struct Sprite* floor_plane;
  // struct Sprite* cloud;
  struct Sprite* flag1;
  struct Sprite* flag2;

  int camera_current_follow_kart;

  int total_markers;
  struct Sprite* marker[MAX_MARKERS];

  int current_npcs;
  struct NPC npcs[MAX_NPCS];

  vec3 starting_pos;
  float starting_heading;

  struct Sprite* trees[MAX_TREES];
  int total_trees;
};

void game_init(struct Game* game, struct Mana* mana, struct Window* window);
void game_delete(struct Game* game, struct Mana* mana);
void game_update(struct Game* game, struct Mana* mana, double delta_time);
void game_render(struct Game* game, struct Mana* mana, double delta_time);
