#include "mana/graphics/utilities/texturemanager/texturemanager.h"

void texture_manager_init(struct TextureManager *texture_manager, struct APICommon *api_common) {
#ifdef VULKAN_API_SUPPORTED
  if (api_common->api_type == API_VULKAN)
    texture_manager->texture_manager_func = VULKAN_TEXTURE_MANAGER;
#endif
#ifdef DIRECTX_12_API_SUPPORTED
  if (api_common->api_type == API_DIRECTX12)
    texture_manager->texture_manager_func = directx_12_TEXTURE_MANAGER;
#endif

  texture_manager->texture_manager_common.texture_index = 0;

  // Note: Store as references because it would be dangerous to realloc in linear memory
  map_init(&(texture_manager->texture_manager_common.textures), sizeof(struct Texture *));

  texture_manager->texture_manager_func.texture_manager_init(&(texture_manager->texture_manager_common), api_common);
}

void texture_manager_delete(struct TextureManager *texture_manager, struct APICommon *api_common) {
  const char *texture_key;
  struct MapIter texture_iter = map_iter();
  while ((texture_key = map_next(&(texture_manager->texture_manager_common.textures), &texture_iter))) {
    struct Texture *texture = *(struct Texture **)map_get(&(texture_manager->texture_manager_common.textures), texture_key);
    texture_delete(texture, api_common);
    free(texture);
  }

  map_delete(&(texture_manager->texture_manager_common.textures));

  texture_manager->texture_manager_func.texture_manager_delete(&(texture_manager->texture_manager_common), api_common);
}

#define MAX_PATH_LENGTH 512

// Convert `wchar_t*` to `char*` (UTF-8) using a stack buffer
static void wchar_to_utf8(const wchar_t *wstr, char *out, size_t out_size) {
  size_t converted_size = 0;
  wcstombs_s(&converted_size, out, out_size, wstr, out_size - 1);
  out[out_size - 1] = '\0';  // Ensure null termination
}

// Convert `char*` (UTF-8) to `wchar_t*` using a stack buffer
static void utf8_to_wchar(const char *str, wchar_t *out, size_t out_size) {
  size_t converted_size = 0;
  mbstowcs_s(&converted_size, out, out_size, str, out_size - 1);
  out[out_size - 1] = L'\0';  // Ensure null termination
}

void texture_manager_add(struct TextureManager *texture_manager, struct APICommon *api_common, struct TextureSettings *texture_settings, wchar_t *path) {
  struct Texture *texture = malloc(sizeof(struct Texture));
  texture_init(texture, &(texture_manager->texture_manager_common), api_common, texture_settings, path, texture_manager->texture_manager_common.texture_index);

  char utf8_path[MAX_PATH_LENGTH];
  wchar_to_utf8(path, utf8_path, sizeof(utf8_path));  // Convert to UTF-8

  map_set(&(texture_manager->texture_manager_common.textures), utf8_path, &texture);  // Store full path in case of models having same texture name like diffuse

  texture_manager->texture_manager_func.texture_manager_add(&(texture_manager->texture_manager_common), api_common, texture_settings);
  texture_manager->texture_manager_common.texture_index++;
}

struct Texture *texture_manager_get(struct TextureManager *texture_manager, wchar_t *texture_name) {
  char utf8_texture_name[MAX_PATH_LENGTH];
  wchar_to_utf8(texture_name, utf8_texture_name, sizeof(utf8_texture_name));  // Convert key to UTF-8

  return *((struct Texture **)map_get(&(texture_manager->texture_manager_common.textures), utf8_texture_name));
}
