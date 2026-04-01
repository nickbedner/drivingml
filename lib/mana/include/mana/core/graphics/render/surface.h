#pragma once

#include "mana/core/graphics/apis/api.h"

struct Surface {
#ifdef _WIN64
  HINSTANCE hinstance;
  HWND hwnd;
  WCHAR class_name[256];
#endif
};
