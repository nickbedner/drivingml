#include "mana/graphics/entities/sprite/spriteanimation.h"

// TODO: Move this to entities?
uint_fast8_t sprite_animation_init(struct SpriteAnimation *sprite_animation, struct APICommon *api_common, struct Shader *shader, struct Texture *texture, size_t num, uint8_t frames, float frame_length, float padding) {
#ifdef VULKAN_API_SUPPORTED
  if (api_common->api_type == API_VULKAN)
    sprite_animation->sprite_animation_func = VULKAN_SPRITE_ANIMATION;
#endif
#ifdef DIRECTX_12_API_SUPPORTED
  if (api_common->api_type == API_DIRECTX12)
    sprite_animation->sprite_animation_func = DIRECTX_12_SPRITE_ANIMATION;
#endif

  sprite_animation->sprite_animation_common.sprite_num = num;

  sprite_animation->sprite_animation_common.direction = -1.0f;
  sprite_animation->sprite_animation_common.animate = true;
  sprite_animation->sprite_animation_common.loop = true;

  sprite_animation->sprite_animation_common.total_frames = frames;
  sprite_animation->sprite_animation_common.frame_length = frame_length;
  sprite_animation->sprite_animation_common.padding = padding;

  sprite_animation->sprite_animation_common.total_animation_length = frame_length * frames;
  sprite_animation->sprite_animation_common.current_animation_time = 0.0f;

  sprite_animation->sprite_animation_common.image_mesh = calloc(1, sizeof(struct Mesh));
  mesh_init(sprite_animation->sprite_animation_common.image_mesh, MESH_TYPE_SPRITE, api_common);

  sprite_animation->sprite_animation_common.image_texture = texture;
  sprite_animation->sprite_animation_common.shader = shader;
  sprite_animation->sprite_animation_common.scale = VEC3_ONE;
  sprite_animation->sprite_animation_common.rotation = QUAT_DEFAULT;

  float tex_norm_width = (float)((texture->texture_common.width / frames) - (padding * 2)) / 100.0f;
  float tex_norm_height = (float)(texture->texture_common.height - (padding * 2)) / 100.0f;

  float tex_norm_width_half = tex_norm_width / 2.0f;
  float tex_norm_height_half = tex_norm_height / 2.0f;

  sprite_animation->sprite_animation_common.width = tex_norm_width;
  sprite_animation->sprite_animation_common.height = tex_norm_height;

  vec3 pos1;
  vec3 pos2;
  vec3 pos3;
  vec3 pos4;

  vec2 uv1;
  vec2 uv2;
  vec2 uv3;
  vec2 uv4;

  float padding_x = (float)padding / (float)texture->texture_common.width;
  float padding_y = (float)padding / (float)texture->texture_common.height;

  if (api_common->inverted_y) {
    pos1 = (vec3){.x = -tex_norm_width_half, .y = -tex_norm_height_half, .z = 0.0f};
    pos2 = (vec3){.x = tex_norm_width_half, .y = -tex_norm_height_half, .z = 0.0f};
    pos3 = (vec3){.x = tex_norm_width_half, .y = tex_norm_height_half, .z = 0.0f};
    pos4 = (vec3){.x = -tex_norm_width_half, .y = tex_norm_height_half, .z = 0.0f};

    uv1 = (vec2){.u = padding_x, .v = padding_y};
    uv2 = (vec2){.u = (1.0f / frames) - padding_x, .v = padding_y};
    uv3 = (vec2){.u = (1.0f / frames) - padding_x, .v = 1.0f - padding_y};
    uv4 = (vec2){.u = padding_x, .v = 1.0f - padding_y};
  } else {
    pos1 = (vec3){.x = -tex_norm_width_half, .y = tex_norm_height_half, .z = 0.0f};
    pos2 = (vec3){.x = tex_norm_width_half, .y = tex_norm_height_half, .z = 0.0f};
    pos3 = (vec3){.x = tex_norm_width_half, .y = -tex_norm_height_half, .z = 0.0f};
    pos4 = (vec3){.x = -tex_norm_width_half, .y = -tex_norm_height_half, .z = 0.0f};

    uv1 = (vec2){.u = padding_x, .v = 1.0f - padding_y};
    uv2 = (vec2){.u = (1.0f / frames) - padding_x, .v = 1.0f - padding_y};
    uv3 = (vec2){.u = (1.0f / frames) - padding_x, .v = padding_y};
    uv4 = (vec2){.u = padding_x, .v = padding_y};
  }

  struct VertexSprite sprite_vertex1 = (struct VertexSprite){.position = (vec3){.x = pos1.x, .y = pos1.y, .z = pos1.z}, .tex_coord = (vec2){.u = uv1.u, .v = uv1.v}};
  struct VertexSprite sprite_vertex2 = (struct VertexSprite){.position = (vec3){.x = pos2.x, .y = pos2.y, .z = pos2.z}, .tex_coord = (vec2){.u = uv2.u, .v = uv2.v}};
  struct VertexSprite sprite_vertex3 = (struct VertexSprite){.position = (vec3){.x = pos3.x, .y = pos3.y, .z = pos3.z}, .tex_coord = (vec2){.u = uv3.u, .v = uv3.v}};
  struct VertexSprite sprite_vertex4 = (struct VertexSprite){.position = (vec3){.x = pos4.x, .y = pos4.y, .z = pos4.z}, .tex_coord = (vec2){.u = uv4.u, .v = uv4.v}};

  mesh_assign_vertex(sprite_animation->sprite_animation_common.image_mesh, &sprite_vertex1);
  mesh_assign_vertex(sprite_animation->sprite_animation_common.image_mesh, &sprite_vertex2);
  mesh_assign_vertex(sprite_animation->sprite_animation_common.image_mesh, &sprite_vertex3);
  mesh_assign_vertex(sprite_animation->sprite_animation_common.image_mesh, &sprite_vertex4);

  mesh_assign_indice(sprite_animation->sprite_animation_common.image_mesh, 0);
  mesh_assign_indice(sprite_animation->sprite_animation_common.image_mesh, 2);
  mesh_assign_indice(sprite_animation->sprite_animation_common.image_mesh, 1);
  mesh_assign_indice(sprite_animation->sprite_animation_common.image_mesh, 0);
  mesh_assign_indice(sprite_animation->sprite_animation_common.image_mesh, 3);
  mesh_assign_indice(sprite_animation->sprite_animation_common.image_mesh, 2);

  sprite_animation->sprite_animation_func.sprite_animation_init(&sprite_animation->sprite_animation_common, api_common, shader, texture);

  return SPRITE_ANIMATION_SUCCESS;
}

void sprite_animation_delete(struct SpriteAnimation *sprite_animation, struct APICommon *api_common) {
  sprite_animation->sprite_animation_func.sprite_animation_delete(&sprite_animation->sprite_animation_common, api_common);

  mesh_delete(sprite_animation->sprite_animation_common.image_mesh, api_common);
  free(sprite_animation->sprite_animation_common.image_mesh);
}

void sprite_animation_render(struct SpriteAnimation *sprite_animation, struct GBufferCommon *gbuffer_common) {
  sprite_animation->sprite_animation_func.sprite_animation_render(&sprite_animation->sprite_animation_common, gbuffer_common);
}

void sprite_animation_update_uniforms(struct SpriteAnimation *sprite_animation, struct APICommon *api_common, struct GBufferCommon *gbuffer_common) {
  sprite_animation->sprite_animation_func.sprite_animation_update_uniforms(&sprite_animation->sprite_animation_common, api_common, gbuffer_common);
}

void sprite_animation_recreate(struct SpriteAnimation *sprite_animation, struct APICommon *api_common, size_t num, uint8_t frames, float frame_length, uint8_t padding) {
  sprite_animation->sprite_animation_func.sprite_animation_delete(&sprite_animation->sprite_animation_common, api_common);
  sprite_animation->sprite_animation_func.sprite_animation_init(&sprite_animation->sprite_animation_common, api_common, sprite_animation->sprite_animation_common.shader, sprite_animation->sprite_animation_common.image_texture);
}

void sprite_animation_update(struct SpriteAnimation *sprite_animation, double delta_time) {
  if (sprite_animation->sprite_animation_common.animate == true) {
    sprite_animation->sprite_animation_common.current_animation_time += (float)delta_time;
    if (sprite_animation->sprite_animation_common.current_animation_time > sprite_animation->sprite_animation_common.total_animation_length) {
      if (sprite_animation->sprite_animation_common.loop == true)
        sprite_animation->sprite_animation_common.current_animation_time = 0.0f;
      else
        sprite_animation->sprite_animation_common.current_animation_time = 0.999f;
    }
  }

  sprite_animation->sprite_animation_common.current_frame = (int32_t)((sprite_animation->sprite_animation_common.current_animation_time / sprite_animation->sprite_animation_common.total_animation_length) * (float)(sprite_animation->sprite_animation_common.total_frames));
  float offset = 0.0f;
  if (sprite_animation->sprite_animation_common.direction < 0.0f)
    offset = 1.0f / (float)(sprite_animation->sprite_animation_common.total_frames);
  sprite_animation->sprite_animation_common.frame_pos = (vec3){.x = ((float)sprite_animation->sprite_animation_common.current_frame / (float)(sprite_animation->sprite_animation_common.total_frames)) + offset, .y = 0.0f, .z = sprite_animation->sprite_animation_common.direction};
}
