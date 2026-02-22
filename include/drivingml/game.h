#pragma once

#include <mana/graphics/entities/sprite/sprite.h>
#include <mana/graphics/shaders/modelshader.h>
#include <mana/graphics/shaders/modelstaticshader.h>
#include <mana/graphics/utilities/camera.h>
#include <mana/graphics/utilities/modelcache.h>
#include <mana/graphics/utilities/spritemanager/spritemanager.h>
#include <mana/graphics/utilities/texturemanager/texturemanager.h>
#include <mana/mana.h>

#include "drivingml/core/player.h"

struct Game {
  struct Window* window;

  struct ArrayList models;

  struct ModelShader model_shader;
  struct ModelStaticShader model_static_shader;

  struct SpriteManager sprite_manager;
  struct TextureManager texture_manager;
  struct ModelCache model_cache;

  struct Player player;

  float mario_speed;
  vec3 mario_position;
  double mario_drive_rotation;
  double mario_drive_accum;
  struct Sprite* water;
  struct Sprite* map;
  struct Sprite* mario;

  struct Sprite* whispy[5];
};

void game_init(struct Game* game, struct Mana* mana, struct Window* window);
void game_delete(struct Game* game, struct Mana* mana);
void game_update(struct Game* game, struct Mana* mana, double delta_time);
