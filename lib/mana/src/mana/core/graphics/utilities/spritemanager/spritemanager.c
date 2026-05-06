#include "mana/core/graphics/utilities/spritemanager/spritemanager.h"

void sprite_manager_init(struct SpriteManager* sprite_manager, struct TextureManager* texture_manager, struct APICommon* api_common, u32 width, u32 height, u8 supersample_scale, struct GBufferCommon* gbuffer_common, u8 msaa_samples, uint_fast32_t descriptors) {
#ifdef VULKAN_API_SUPPORTED
  if (api_common->api_type == API_VULKAN)
    sprite_manager->sprite_manager_func = VULKAN_SPRITE_MANAGER;
#endif
#ifdef DIRECTX_12_API_SUPPORTED
  if (api_common->api_type == API_DIRECTX_12)
    sprite_manager->sprite_manager_func = DIRECTX_12_SPRITE_MANAGER;
#endif

  sprite_shader_init(&(sprite_manager->sprite_manager_common.sprite_shader), api_common, width, height, supersample_scale, gbuffer_common, TRUE, FALSE, msaa_samples, descriptors);

  array_list_init(&(sprite_manager->sprite_manager_common.sprites));

  sprite_manager->sprite_manager_common.texture_manager = texture_manager;

  sprite_manager->sprite_manager_func.sprite_manager_init_sprite_pool(&(sprite_manager->sprite_manager_common), api_common, width, height, supersample_scale, gbuffer_common, msaa_samples, descriptors);
}

void sprite_manager_delete(struct SpriteManager* sprite_manager, struct APICommon* api_common) {
  for (size_t sprite_num = 0; sprite_num < array_list_size(&(sprite_manager->sprite_manager_common.sprites)); sprite_num++) {
    struct Sprite* sprite = (struct Sprite*)array_list_get(&(sprite_manager->sprite_manager_common.sprites), sprite_num);
    sprite_delete(sprite, api_common);
    free(sprite);
  }

  array_list_delete(&(sprite_manager->sprite_manager_common.sprites));
  array_list_delete(&(sprite_manager->sprite_manager_common.sprites_animations));

  sprite_shader_delete(&(sprite_manager->sprite_manager_common.sprite_shader), api_common);

  sprite_manager->sprite_manager_func.sprite_manager_delete_sprite(&(sprite_manager->sprite_manager_common), api_common);
}

void sprite_manager_resize(struct SpriteManager* sprite_manager, struct APICommon* api_common, uint_fast32_t width, uint_fast32_t height, u8 supersample_scale) {
  shader_resize(&(sprite_manager->sprite_manager_common.sprite_shader.shader), api_common, width, height, supersample_scale);
}

struct Sprite* sprite_manager_add_sprite(struct SpriteManager* sprite_manager, struct APICommon* api_common, const char* texture_name) {
  // Note: These would be pooled and preallocated I'm guessing
  struct Sprite* sprite = (struct Sprite*)calloc(1, sizeof(struct Sprite));
  size_t sprite_num = array_list_size(&(sprite_manager->sprite_manager_common.sprites));
  sprite_manager->sprite_manager_func.sprite_manager_add_sprite(&(sprite_manager->sprite_manager_common), api_common, sprite, sprite_num);
  sprite_init(sprite, api_common, &(sprite_manager->sprite_manager_common.sprite_shader.shader), texture_manager_get(sprite_manager->sprite_manager_common.texture_manager, texture_name), sprite_num);

  array_list_add(&(sprite_manager->sprite_manager_common.sprites), sprite);

  return sprite;
}

void sprite_manager_remove(struct SpriteManager* sprite_manager, struct APICommon* api_common, size_t sprite_num) {
  struct Sprite* moved_sprite = (struct Sprite*)array_list_get(&(sprite_manager->sprite_manager_common.sprites), array_list_size(&(sprite_manager->sprite_manager_common.sprites)) - 1);
  moved_sprite->sprite_common.sprite_num = sprite_num;
  array_list_swap(&(sprite_manager->sprite_manager_common.sprites), sprite_num, array_list_size(&(sprite_manager->sprite_manager_common.sprites)) - 1);
  struct Sprite* old_sprite = (struct Sprite*)array_list_pop_back(&(sprite_manager->sprite_manager_common.sprites));
  sprite_delete(old_sprite, api_common);
}

void sprite_manager_update_uniforms(struct SpriteManager* sprite_manager, struct APICommon* api_common, struct GBufferCommon* gbuffer_common) {
  for (size_t entity_num = 0; entity_num < array_list_size(&(sprite_manager->sprite_manager_common.sprites)); entity_num++)
    sprite_update_uniforms((struct Sprite*)array_list_get(&(sprite_manager->sprite_manager_common.sprites), entity_num), api_common, gbuffer_common);
}

internal inline r64 sprite_depth(const struct Sprite* sprite, vec4d sort_key) {
  vec3d p = vec3_to_vec3d(sprite->sprite_common.position);
  return p.x * sort_key.x + p.y * sort_key.y + p.z * sort_key.z + sort_key.w;
}

internal void sprite_insertion_sort(struct Sprite** sprites, size_t count, vec4d sort_key) {
  for (size_t i = 1; i < count; i++) {
    struct Sprite* key = sprites[i];
    r64 key_depth = sprite_depth(key, sort_key);

    size_t j = i;

    while (j > 0) {
      r64 prev_depth = sprite_depth(sprites[j - 1], sort_key);

      if (prev_depth < key_depth) {
        sprites[j] = sprites[j - 1];
        j--;
      } else
        break;
    }

    sprites[j] = key;
  }
}

void sprite_manager_render(struct SpriteManager* sprite_manager, struct GBufferCommon* gbuffer_common, vec4d sort_key) {
  size_t count = array_list_size(&(sprite_manager->sprite_manager_common.sprites));

  struct Sprite* ordered_sprites[128] = {0};

  if (count > 128)
    count = 128;

  for (size_t i = 0; i < count; i++)
    ordered_sprites[i] = (struct Sprite*)array_list_get(&sprite_manager->sprite_manager_common.sprites, i);
  sprite_insertion_sort(ordered_sprites, count, sort_key);
  for (size_t i = 0; i < count; i++)
    sprite_render(ordered_sprites[i], gbuffer_common);
}

void sprite_manager_update(struct SpriteManager* sprite_manager, r64 delta_time) {
}
