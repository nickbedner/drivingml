#include "mana/graphics/utilities/texture/texture.h"

// TODO: This needs to have missing file error check
uint8_t texture_init(struct Texture* texture, struct TextureManagerCommon* texture_manager_common, struct APICommon* api_common, struct TextureSettings texture_settings, const char* path, size_t texture_index) {
  struct TextureCommon* texture_common = &(texture->texture_common);
  texture_common->texture_settings = texture_settings;

  texture_common->path = strdup(path);

  char* name_location = strrchr(texture_common->path, '/');
#ifdef _WIN32
  if (!name_location) name_location = strrchr(texture_common->path, '\\');
#endif
  texture_common->name = name_location ? strdup(name_location + 1) : strdup(texture_common->path);

  char* type_location = strrchr(texture_common->path, '.');
  texture_common->type = type_location ? strdup(type_location + 1) : strdup("");

  texture_common->texture_manager_common = texture_manager_common;
  texture_common->id = texture_index;

  uint32_t w0 = 0, h0 = 0, ch0 = 0;
  uint8_t bit0 = 0, ct0 = 0;

  void* pixels0 = texture_read_png(texture_common->path, api_common->asset_directory, &w0, &h0, &ch0, &bit0, &ct0);

  if (!pixels0) {
    log_message(LOG_SEVERITY_WARNING, "Failed to load texture image: %s\n", texture_common->path);

    return 1;
  }

  // If ct0 == 2, pixels0 is now RGBA -> force channels=4 so sizes match GPU upload.
  if (ct0 == 2) ch0 = 4;

  // Pick format from bit depth + channels
  switch (bit0) {
    case 8:
      texture_common->texture_settings.format_type = (ch0 == 1) ? FORMAT_R8_UNORM : FORMAT_R8G8B8A8_UNORM;
      break;
    case 16:
      texture_common->texture_settings.format_type = (ch0 == 1) ? FORMAT_R16_UNORM : FORMAT_R16G16B16A16_UNORM;
      break;
    default:
      log_message(LOG_SEVERITY_WARNING, "Unsupported bit depth: %d\n", bit0);
      return 1;
  }

  texture_common->width = w0;
  texture_common->height = h0;
  texture_common->channels = ch0;
  texture_common->bit_depth = bit0;

#ifdef VULKAN_API_SUPPORTED
  if (api_common->api_type == API_VULKAN) texture->texture_func = VULKAN_TEXTURE;
#endif
#ifdef DIRECTX_12_API_SUPPORTED
  if (api_common->api_type == API_DIRECTX12) texture->texture_func = directx_12_TEXTURE;
#endif

  uint8_t result = texture->texture_func.texture_init(&(texture->texture_common), texture_common->texture_manager_common, api_common, pixels0);

  free(pixels0);

  return result;
}

void texture_delete(struct Texture* texture, struct APICommon* api_common) {
  texture->texture_func.texture_delete(&(texture->texture_common), api_common);

  free(texture->texture_common.path);
  free(texture->texture_common.name);
  free(texture->texture_common.type);
}
