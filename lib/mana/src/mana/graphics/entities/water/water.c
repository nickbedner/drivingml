#include "mana/graphics/entities/water/water.h"

u8 water_init(struct Water* water, struct APICommon* api_common, struct Shader* shader, struct Texture* texture) {
#ifdef VULKAN_API_SUPPORTED
  if (api_common->api_type == API_VULKAN)
    water->water_func = VULKAN_WATER;
#endif
#ifdef DIRECTX_12_API_SUPPORTED
  if (api_common->api_type == API_DIRECTX_12)
    water->water_func = DIRECTX_12_WATER;
#endif

  water->water_common.water_mesh = (struct Mesh*)calloc(1, sizeof(struct Mesh));
  mesh_init(water->water_common.water_mesh, MESH_TYPE_QUAD, api_common);

  water->water_common.wave_texture = texture;
  water->water_common.shader = shader;
  water->water_common.scale = VEC3_ONE;
  water->water_common.rotation = QUAT_DEFAULT;
  water->water_common.time = 0.0f;

  r32 width = 1.0f;
  r32 height = 1.0f;

  r32 half_w = width * 0.5f;
  r32 half_h = height * 0.5f;

  vec3 pos1 = {.x = -half_w, .y = 0.0f, .z = -half_h};
  vec3 pos2 = {.x = half_w, .y = 0.0f, .z = -half_h};
  vec3 pos3 = {.x = half_w, .y = 0.0f, .z = half_h};
  vec3 pos4 = {.x = -half_w, .y = 0.0f, .z = half_h};

  vec2 uv1 = {.x = 0.0f, .y = 0.0f};
  vec2 uv2 = {.x = 1.0f, .y = 0.0f};
  vec2 uv3 = {.x = 1.0f, .y = 1.0f};
  vec2 uv4 = {.x = 0.0f, .y = 1.0f};

  struct VertexSprite v1 = {pos1, uv1};
  struct VertexSprite v2 = {pos2, uv2};
  struct VertexSprite v3 = {pos3, uv3};
  struct VertexSprite v4 = {pos4, uv4};

  mesh_assign_vertex(water->water_common.water_mesh, &v1);
  mesh_assign_vertex(water->water_common.water_mesh, &v2);
  mesh_assign_vertex(water->water_common.water_mesh, &v3);
  mesh_assign_vertex(water->water_common.water_mesh, &v4);

  mesh_assign_indice(water->water_common.water_mesh, 0);
  mesh_assign_indice(water->water_common.water_mesh, 2);
  mesh_assign_indice(water->water_common.water_mesh, 1);
  mesh_assign_indice(water->water_common.water_mesh, 0);
  mesh_assign_indice(water->water_common.water_mesh, 3);
  mesh_assign_indice(water->water_common.water_mesh, 2);

  water->water_func.water_init(&water->water_common, api_common, shader, texture);

  return 1;
}

void water_delete(struct Water* water, struct APICommon* api_common) {
  water->water_func.water_delete(&water->water_common, api_common);

  mesh_delete(water->water_common.water_mesh, api_common);
  free(water->water_common.water_mesh);
}

void water_render(struct Water* water, struct GBufferCommon* gbuffer_common) {
  water->water_func.water_render(&water->water_common, gbuffer_common);
}

void water_update_uniforms(struct Water* water, struct APICommon* api_common, struct GBufferCommon* gbuffer_common, u32 width, u32 height) {
  water->water_common.time += 0.016f;  // replace with delta time later
  water->water_func.water_update_uniforms(&water->water_common, api_common, gbuffer_common, width, height);
}
