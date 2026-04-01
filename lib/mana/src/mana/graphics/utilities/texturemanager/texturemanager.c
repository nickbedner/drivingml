#include "mana/graphics/utilities/texturemanager/texturemanager.h"

void texture_manager_init(struct TextureManager* texture_manager, struct APICommon* api_common) {
#ifdef VULKAN_API_SUPPORTED
  if (api_common->api_type == API_VULKAN)
    texture_manager->texture_manager_func = VULKAN_TEXTURE_MANAGER;
#endif
#ifdef DIRECTX_12_API_SUPPORTED
  if (api_common->api_type == API_DIRECTX_12)
    texture_manager->texture_manager_func = directx_12_TEXTURE_MANAGER;
#endif

  texture_manager->texture_manager_common.texture_index = 0;

  // Note: Store as references because it would be dangerous to realloc in linear memory
  map_init(&(texture_manager->texture_manager_common.textures), sizeof(struct Texture*));

  texture_manager->texture_manager_func.texture_manager_init(&(texture_manager->texture_manager_common), api_common);
}

void texture_manager_delete(struct TextureManager* texture_manager, struct APICommon* api_common) {
  const char* texture_key;
  struct MapIter texture_iter = map_iter();
  while ((texture_key = map_next(&(texture_manager->texture_manager_common.textures), &texture_iter))) {
    struct Texture* texture = *(struct Texture**)map_get(&(texture_manager->texture_manager_common.textures), texture_key);
    texture_delete(texture, api_common);
    free(texture);
  }

  map_delete(&(texture_manager->texture_manager_common.textures));

  texture_manager->texture_manager_func.texture_manager_delete(&(texture_manager->texture_manager_common), api_common);
}

void texture_manager_add(struct TextureManager* texture_manager, struct APICommon* api_common, struct TextureSettings texture_settings, const char* path, b8 is_sprite) {
  struct Texture* texture = (struct Texture*)malloc(sizeof(struct Texture));
  texture_init(texture, &(texture_manager->texture_manager_common), api_common, texture_settings, path, texture_manager->texture_manager_common.texture_index, is_sprite);

  map_set(&(texture_manager->texture_manager_common.textures), path, &texture);  // Store full path in case of models having same texture name like diffuse

  texture_manager->texture_manager_func.texture_manager_add(&(texture_manager->texture_manager_common), api_common, texture_settings);
  texture_manager->texture_manager_common.texture_index++;
}

void texture_manager_add_array(struct TextureManager* texture_manager, struct APICommon* api_common, struct TextureSettings texture_settings, const char* texture_name, const char* const* paths, u32 layer_count) {
  if (!texture_name || !paths || layer_count == 0) {
    log_message(LOG_SEVERITY_WARNING, "texture_manager_add_array got invalid args\n");
    return;
  }

  struct Texture* texture = (struct Texture*)malloc(sizeof(struct Texture));
  if (!texture) {
    log_message(LOG_SEVERITY_ERROR, "Failed to allocate texture\n");
    return;
  }

  memset(texture, 0, sizeof(struct Texture));

  u8 result = texture_array_init(texture, &(texture_manager->texture_manager_common), api_common, texture_settings, paths, layer_count, texture_manager->texture_manager_common.texture_index);

  if (result != 0) {
    free(texture);
    return;
  }

  map_set(&(texture_manager->texture_manager_common.textures), texture_name, &texture);

  texture_manager->texture_manager_func.texture_manager_add(&(texture_manager->texture_manager_common), api_common, texture_settings);

  texture_manager->texture_manager_common.texture_index++;
}

struct Texture* texture_manager_get(struct TextureManager* texture_manager, const char* texture_name) {
  return *((struct Texture**)map_get(&(texture_manager->texture_manager_common.textures), texture_name));
}
