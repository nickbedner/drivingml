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

  struct Sprite* track;
  struct Sprite* floor_plane;
  struct Sprite* flag1;
  struct Sprite* flag2;

  struct Sprite* marker[MAX_MARKERS];
  struct Sprite* trees[MAX_TREES];

  struct Model* test_model;

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

  float previous_reward;
  float starting_heading;

  int start_timer;
  int timer;

  int camera_current_follow_kart;

  int total_markers;
  int current_npcs;
  int total_trees;
};

void game_init(struct Game* game, struct Mana* mana, struct Window* window);
void game_delete(struct Game* game, struct Mana* mana);
void game_update(struct Game* game, struct Mana* mana, double delta_time);
void game_render(struct Game* game, struct Mana* mana, double delta_time);
