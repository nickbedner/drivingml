#include "mana/core/graphics/utilities/texture/texture.h"

// TODO: This needs to have missing file error check
u8 texture_init(struct Texture* texture, struct TextureManagerCommon* texture_manager_common, struct APICommon* api_common, struct TextureSettings texture_settings, const char* path, size_t texture_index, b8 is_sprite) {
  struct TextureCommon* texture_common = &(texture->texture_common);
  texture_common->texture_settings = texture_settings;
  texture_common->layer_count = 1;
  texture_common->is_array = is_sprite;

  texture_common->path = _strdup(path);

  char* name_location = strrchr(texture_common->path, '/');
#ifdef _WIN32
  if (!name_location) name_location = strrchr(texture_common->path, '\\');
#endif
  texture_common->name = name_location ? _strdup(name_location + 1) : _strdup(texture_common->path);

  char* type_location = strrchr(texture_common->path, '.');
  texture_common->type = type_location ? _strdup(type_location + 1) : _strdup("");

  texture_common->texture_manager_common = texture_manager_common;
  texture_common->id = texture_index;
  texture_common->dimension = TEXTURE_DIMENSION_2D;

  u32 w0 = 0, h0 = 0, ch0 = 0;
  u8 bit0 = 0, ct0 = 0;

  void* pixels0 = png_loader_read_png(texture_common->path, api_common->asset_directory, &w0, &h0, &ch0, &bit0, &ct0);

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
  if (api_common->api_type == API_DIRECTX_12) texture->texture_func = DIRECTX_12_TEXTURE;
#endif

  u8 result = texture->texture_func.texture_init(&(texture->texture_common), texture_common->texture_manager_common, api_common, pixels0);

  free(pixels0);

  return result;
}

u8 texture_array_init(struct Texture* texture, struct TextureManagerCommon* texture_manager_common, struct APICommon* api_common, struct TextureSettings texture_settings, const char* const* paths, u32 layer_count, size_t texture_index) {
  if (!texture || !paths || layer_count == 0) {
    log_message(LOG_SEVERITY_WARNING, "texture_array_init got invalid args\n");
    return 1;
  }

  struct TextureCommon* texture_common = &(texture->texture_common);
  memset(texture_common, 0, sizeof(*texture_common));

  texture_common->texture_settings = texture_settings;
  texture_common->texture_manager_common = texture_manager_common;
  texture_common->id = texture_index;
  texture_common->is_array = TRUE;

  // For now store the first path as the texture path/name/type source.
  texture_common->path = _strdup(paths[0]);
  if (!texture_common->path) {
    log_message(LOG_SEVERITY_ERROR, "Failed to alloc texture path\n");
    return 1;
  }

  char* name_location = strrchr(texture_common->path, '/');
#ifdef _WIN32
  if (!name_location) name_location = strrchr(texture_common->path, '\\');
#endif
  texture_common->name = name_location ? _strdup(name_location + 1) : _strdup(texture_common->path);

  char* type_location = strrchr(texture_common->path, '.');
  texture_common->type = type_location ? _strdup(type_location + 1) : _strdup("");

  if (!texture_common->name || !texture_common->type) {
    free(texture_common->path);
    free(texture_common->name);
    free(texture_common->type);
    log_message(LOG_SEVERITY_ERROR, "Failed to alloc texture metadata\n");
    return 1;
  }

  // Minimum version: keep mip arrays out until layered upload works.
  if (texture_settings.mip_type == MIP_CUSTOM || texture_settings.mip_type == MIP_GENERATE) {
    log_message(LOG_SEVERITY_WARNING, "texture_array_init currently only supports non-mipped textures\n");
    free(texture_common->path);
    free(texture_common->name);
    free(texture_common->type);
    return 1;
  }

  texture_common->layer_count = layer_count;
  texture_common->mip_levels = 1;
  texture_common->dimension = TEXTURE_DIMENSION_2D_ARRAY;

  u32 w0 = 0, h0 = 0, ch0 = 0;
  u8 bit0 = 0, ct0 = 0;

  void* pixels0 = png_loader_read_png(paths[0], api_common->asset_directory, &w0, &h0, &ch0, &bit0, &ct0);
  if (!pixels0) {
    log_message(LOG_SEVERITY_WARNING, "Failed to load texture array frame 0: %s\n", paths[0]);
    free(texture_common->path);
    free(texture_common->name);
    free(texture_common->type);
    return 1;
  }

  // Match your current single-texture rule.
  if (ct0 == 2) ch0 = 4;

  switch (bit0) {
    case 8:
      texture_common->texture_settings.format_type = (ch0 == 1) ? FORMAT_R8_UNORM : FORMAT_R8G8B8A8_UNORM;
      break;
    case 16:
      texture_common->texture_settings.format_type = (ch0 == 1) ? FORMAT_R16_UNORM : FORMAT_R16G16B16A16_UNORM;
      break;
    default:
      log_message(LOG_SEVERITY_WARNING, "Unsupported bit depth in texture array: %u\n", bit0);
      free(pixels0);
      free(texture_common->path);
      free(texture_common->name);
      free(texture_common->type);
      return 1;
  }

  texture_common->width = w0;
  texture_common->height = h0;
  texture_common->channels = ch0;
  texture_common->bit_depth = bit0;

  size_t bytes_per_channel = (bit0 == 16) ? 2 : 1;
  size_t frame_size = (size_t)w0 * (size_t)h0 * (size_t)ch0 * bytes_per_channel;
  size_t total_size = frame_size * (size_t)layer_count;

  u8* combined_pixels = (u8*)malloc(total_size);
  if (!combined_pixels) {
    log_message(LOG_SEVERITY_ERROR, "Failed to alloc texture array upload buffer\n");
    free(pixels0);
    free(texture_common->path);
    free(texture_common->name);
    free(texture_common->type);
    return 1;
  }

  memcpy(combined_pixels, pixels0, frame_size);
  free(pixels0);

  for (u32 layer = 1; layer < layer_count; layer++) {
    u32 w = 0, h = 0, ch = 0;
    u8 bit = 0, ct = 0;

    void* pixels = png_loader_read_png(paths[layer], api_common->asset_directory, &w, &h, &ch, &bit, &ct);
    if (!pixels) {
      log_message(LOG_SEVERITY_WARNING, "Failed to load texture array frame %u: %s\n", layer, paths[layer]);
      free(combined_pixels);
      free(texture_common->path);
      free(texture_common->name);
      free(texture_common->type);
      return 1;
    }

    if (ct == 2) ch = 4;

    if (w != w0 || h != h0 || ch != ch0 || bit != bit0) {
      log_message(LOG_SEVERITY_WARNING,
                  "Texture array frame %u mismatch. Got %ux%u ch=%u bit=%u, expected %ux%u ch=%u bit=%u\n",
                  layer, w, h, ch, bit, w0, h0, ch0, bit0);
      free(pixels);
      free(combined_pixels);
      free(texture_common->path);
      free(texture_common->name);
      free(texture_common->type);
      return 1;
    }

    memcpy(combined_pixels + ((size_t)layer * frame_size), pixels, frame_size);
    free(pixels);
  }

#ifdef VULKAN_API_SUPPORTED
  if (api_common->api_type == API_VULKAN) texture->texture_func = VULKAN_TEXTURE;
#endif
#ifdef DIRECTX_12_API_SUPPORTED
  if (api_common->api_type == API_DIRECTX_12) texture->texture_func = DIRECTX_12_TEXTURE;
#endif

  u8 result = texture->texture_func.texture_init(&(texture->texture_common), texture_common->texture_manager_common, api_common, combined_pixels);

  free(combined_pixels);

  if (result != 0) {
    free(texture_common->path);
    free(texture_common->name);
    free(texture_common->type);
  }

  return result;
}

void texture_delete(struct Texture* texture, struct APICommon* api_common) {
  texture->texture_func.texture_delete(&(texture->texture_common), api_common);

  free(texture->texture_common.path);
  free(texture->texture_common.name);
  free(texture->texture_common.type);
}
