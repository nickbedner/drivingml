#pragma once

#include <time.h>

#include "mana/graphics/apis/api.h"
#include "mana/graphics/render/window.h"

enum MANA_STATUS {
  MANA_SUCCESS = 0,
  MANA_GPU_API_ERROR,
  MANA_LAST_ERROR
};

// Note: We'll leave this simple because we might add running stuff like audio engine, physics engine, etc.
struct Mana {
  struct API api;
};

u8 mana_init(struct Mana* mana, enum API_TYPE preferred_api_type);
void mana_delete(struct Mana* mana);
