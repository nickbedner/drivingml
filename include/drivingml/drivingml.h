#pragma once

#define PROJECT_NAME "DrivingML"

#include <mana/mana.h>

#include "game.h"

struct DrivingML {
  struct Mana mana;
  struct Window window;
  struct Game game;
};

enum DRIVINGML_STATUS {
  DRIVINGML_SUCCESS = 0,
  DRIVINGML_MANA_ERROR,
  DRIVINGML_WINDOW_ERROR,
  DRIVINGML_LAST_ERROR
};

u8 drivingml_init(struct DrivingML* drivingml);
void drivingml_delete(struct DrivingML* drivingml);
void drivingml_start(struct DrivingML* drivingml);
