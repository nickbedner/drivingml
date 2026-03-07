#pragma once

#include <mana/graphics/entities/sprite/sprite.h>
#include <mana/graphics/shaders/modelshader.h>
#include <mana/graphics/shaders/modelstaticshader.h>
#include <mana/graphics/utilities/camera.h>
#include <mana/graphics/utilities/modelcache.h>
#include <mana/graphics/utilities/spritemanager/spritemanager.h>
#include <mana/graphics/utilities/texturemanager/texturemanager.h>
#include <mana/mana.h>
#include <mana/utilities/xmlparser.h>

#include "drivingml/core/ac_model.h"
#include "drivingml/core/player.h"

#define EVAL_MODE false

#define MAX_MARKERS 32
#define MAX_NPCS 8

struct NPC {
  ACModel model;
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
  struct Sprite* start;
  struct Sprite* finish;
  struct Sprite* fence;

  int total_markers;
  struct Sprite* marker[MAX_MARKERS];

  int current_npcs;
  struct NPC npcs[MAX_NPCS];
};

void game_init(struct Game* game, struct Mana* mana, struct Window* window);
void game_delete(struct Game* game, struct Mana* mana);
void game_update(struct Game* game, struct Mana* mana, double delta_time);
