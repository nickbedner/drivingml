#pragma once

#include "mana/graphics/apis/api.h"

struct Surface {
#ifdef _WIN64
  HINSTANCE hinstance;
  HWND hwnd;
  wchar_t class_name[256];
#endif
};
