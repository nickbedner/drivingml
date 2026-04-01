#include "mana/core/graphics/entities/sprite/sprite.h"

u8 sprite_init(struct Sprite* sprite, struct APICommon* api_common, struct Shader* shader, struct Texture* texture, size_t num) {
#ifdef VULKAN_API_SUPPORTED
  if (api_common->api_type == API_VULKAN)
    sprite->sprite_func = VULKAN_SPRITE;
#endif
#ifdef DIRECTX_12_API_SUPPORTED
  if (api_common->api_type == API_DIRECTX_12)
    sprite->sprite_func = DIRECTX_12_SPRITE;
#endif

  sprite->sprite_common.sprite_num = num;

  sprite->sprite_common.image_mesh = (struct Mesh*)calloc(1, sizeof(struct Mesh));
  mesh_init(sprite->sprite_common.image_mesh, MESH_TYPE_SPRITE, api_common);

  sprite->sprite_common.image_texture = texture;
  sprite->sprite_common.shader = shader;
  sprite->sprite_common.scale = VEC3_ONE;
  sprite->sprite_common.rotation = QUAT_DEFAULT;

  r32 tex_norm_width = (r32)(texture->texture_common.width) / 100.0f;
  r32 tex_norm_height = (r32)(texture->texture_common.height) / 100.0f;

  r32 tex_norm_width_half = tex_norm_width / 2.0f;
  r32 tex_norm_height_half = tex_norm_height / 2.0f;

  sprite->sprite_common.width = tex_norm_width;
  sprite->sprite_common.height = tex_norm_height;

  vec3 pos1;
  vec3 pos2;
  vec3 pos3;
  vec3 pos4;

  vec2 uv1;
  vec2 uv2;
  vec2 uv3;
  vec2 uv4;

  pos1 = (vec3){.x = -tex_norm_width_half, .y = tex_norm_height_half, .z = 0.0f};
  pos2 = (vec3){.x = tex_norm_width_half, .y = tex_norm_height_half, .z = 0.0f};
  pos3 = (vec3){.x = tex_norm_width_half, .y = -tex_norm_height_half, .z = 0.0f};
  pos4 = (vec3){.x = -tex_norm_width_half, .y = -tex_norm_height_half, .z = 0.0f};

  if (api_common->inverted_y) {
    uv1 = (vec2){.u = 0.0f, .v = 0.0f};
    uv2 = (vec2){.u = 1.0f, .v = 0.0f};
    uv3 = (vec2){.u = 1.0f, .v = 1.0f};
    uv4 = (vec2){.u = 0.0f, .v = 1.0f};
  } else {
    uv1 = (vec2){.u = 0.0f, .v = 1.0f};
    uv2 = (vec2){.u = 1.0f, .v = 1.0f};
    uv3 = (vec2){.u = 1.0f, .v = 0.0f};
    uv4 = (vec2){.u = 0.0f, .v = 0.0f};
  }

  struct VertexSprite sprite_vertex1 = (struct VertexSprite){.position = (vec3){.x = pos1.x, .y = pos1.y, .z = pos1.z}, .tex_coord = (vec2){.u = uv1.u, .v = uv1.v}};
  struct VertexSprite sprite_vertex2 = (struct VertexSprite){.position = (vec3){.x = pos2.x, .y = pos2.y, .z = pos2.z}, .tex_coord = (vec2){.u = uv2.u, .v = uv2.v}};
  struct VertexSprite sprite_vertex3 = (struct VertexSprite){.position = (vec3){.x = pos3.x, .y = pos3.y, .z = pos3.z}, .tex_coord = (vec2){.u = uv3.u, .v = uv3.v}};
  struct VertexSprite sprite_vertex4 = (struct VertexSprite){.position = (vec3){.x = pos4.x, .y = pos4.y, .z = pos4.z}, .tex_coord = (vec2){.u = uv4.u, .v = uv4.v}};

  mesh_assign_vertex(sprite->sprite_common.image_mesh, &sprite_vertex1);
  mesh_assign_vertex(sprite->sprite_common.image_mesh, &sprite_vertex2);
  mesh_assign_vertex(sprite->sprite_common.image_mesh, &sprite_vertex3);
  mesh_assign_vertex(sprite->sprite_common.image_mesh, &sprite_vertex4);

  mesh_assign_indice(sprite->sprite_common.image_mesh, 0);
  mesh_assign_indice(sprite->sprite_common.image_mesh, 2);
  mesh_assign_indice(sprite->sprite_common.image_mesh, 1);
  mesh_assign_indice(sprite->sprite_common.image_mesh, 0);
  mesh_assign_indice(sprite->sprite_common.image_mesh, 3);
  mesh_assign_indice(sprite->sprite_common.image_mesh, 2);

  sprite->sprite_func.sprite_init(&sprite->sprite_common, api_common, shader, texture, num);

  return SPRITE_SUCCESS;
}

void sprite_delete(struct Sprite* sprite, struct APICommon* api_common) {
  sprite->sprite_func.sprite_delete(&sprite->sprite_common, api_common);

  mesh_delete(sprite->sprite_common.image_mesh, api_common);
  free(sprite->sprite_common.image_mesh);
}

void sprite_render(struct Sprite* sprite, struct GBufferCommon* gbuffer_common) {
  sprite->sprite_func.sprite_render(&sprite->sprite_common, gbuffer_common);
}

void sprite_update_uniforms(struct Sprite* sprite, struct APICommon* api_common, struct GBufferCommon* gbuffer_common) {
  sprite->sprite_func.sprite_update_uniforms(&sprite->sprite_common, api_common, gbuffer_common);
}

void sprite_recreate(struct Sprite* sprite, struct APICommon* api_common, size_t num) {
  sprite->sprite_func.sprite_delete(&sprite->sprite_common, api_common);
  sprite->sprite_func.sprite_init(&sprite->sprite_common, api_common, sprite->sprite_common.shader, sprite->sprite_common.image_texture, num);
}
