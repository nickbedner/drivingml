#pragma once

#include "mana/graphics/apis/api.h"
#ifdef VULKAN_API_SUPPORTED
#include "mana/graphics/utilities/mesh/meshvulkan.h"
#endif
#ifdef DIRECTX_12_API_SUPPORTED
#include "mana/graphics/utilities/mesh/meshdirectx12.h"
#endif

struct MeshFunc {
  u8 (*mesh_init)(struct MeshCommon*, enum MESH_TYPE, struct APICommon*);
  void (*mesh_delete)(struct MeshCommon*, struct APICommon*);
  void (*mesh_generate_buffers)(struct MeshCommon*, struct APICommon*);
};

#ifdef VULKAN_API_SUPPORTED
internal const struct MeshFunc MESH_VULKAN = {mesh_vulkan_init, mesh_vulkan_delete, mesh_vulkan_generate_buffers};
#endif
#ifdef DIRECTX_12_API_SUPPORTED
internal const struct MeshFunc MESH_DIRECTX12 = {mesh_directx_12_init, mesh_directx_12_delete, mesh_directx_12_generate_buffers};
#endif

struct Mesh {
  struct MeshFunc mesh_func;
  struct MeshCommon mesh_common;
};

u8 mesh_init(struct Mesh* mesh, enum MESH_TYPE mesh_type, struct APICommon* api_common);
void mesh_delete(struct Mesh* mesh, struct APICommon* api_common);
u32 mesh_get_memory_size(enum MESH_TYPE mesh_type);
void mesh_generate_buffers(struct Mesh* mesh, struct APICommon* api_common);
void mesh_clear(struct Mesh* mesh);
void mesh_clear_vertices(struct Mesh* mesh);
void mesh_clear_indices(struct Mesh* mesh);
void mesh_assign_vertex(struct Mesh* mesh, void* vertex);
void mesh_assign_indice(struct Mesh* mesh, u32 indice);

void mesh_fullscreen_triangle(struct Mesh* mesh);

// internal inline void mesh_sprite_init(struct Mesh *mesh);
// internal inline void mesh_sprite_assign_vertex(struct Vector *vector, r32 x, r32 y, r32 z, r32 u, r32 v);
// internal inline VkVertexInputBindingDescription mesh_sprite_get_binding_description(void);
// internal inline void mesh_sprite_get_attribute_descriptions(VkVertexInputAttributeDescription *attribute_descriptions);
//
// internal inline void mesh_quad_init(struct Mesh *mesh);
// internal inline void mesh_quad_assign_vertex(struct Vector *vector, r32 x, r32 y, r32 z);
// internal inline VkVertexInputBindingDescription mesh_quad_get_binding_description(void);
// internal inline void mesh_quad_get_attribute_descriptions(VkVertexInputAttributeDescription *attribute_descriptions);
//
// internal inline void mesh_triangle_init(struct Mesh *mesh);
// internal inline void mesh_triangle_assign_vertex(struct Vector *vector, r32 x, r32 y, r32 z);
// internal inline VkVertexInputBindingDescription mesh_triangle_get_binding_description(void);
// internal inline void mesh_triangle_get_attribute_descriptions(VkVertexInputAttributeDescription *attribute_descriptions);
//
// internal inline void mesh_model_init(struct Mesh *mesh);
// internal inline void mesh_model_assign_vertex(struct Vector *vector, r32 x, r32 y, r32 z, r32 r1, r32 g1, r32 b1, r32 u, r32 v, r32 r2, r32 g2, r32 b2, int joint_id_x, int joint_id_y, int joint_id_z, r32 weight_x, r32 weight_y, r32 weight_z);
// internal inline VkVertexInputBindingDescription mesh_model_get_binding_description(void);
// internal inline void mesh_model_get_attribute_descriptions(VkVertexInputAttributeDescription *attribute_descriptions);
//
// internal inline void mesh_model_internal_init(struct Mesh *mesh);
// internal inline void mesh_model_internal_assign_vertex(struct Vector *vector, r32 x, r32 y, r32 z, r32 r1, r32 g1, r32 b1, r32 u, r32 v, r32 r2, r32 g2, r32 b2);
// internal inline VkVertexInputBindingDescription mesh_model_internal_get_binding_description(void);
// internal inline void mesh_model_internal_get_attribute_descriptions(VkVertexInputAttributeDescription *attribute_descriptions);
//
// internal inline void mesh_dual_contouring_init(struct Mesh *mesh);
// internal inline void mesh_dual_contouring_assign_vertex(struct Vector *vector, r32 x, r32 y, r32 z, r32 r, r32 g, r32 b);
// internal inline VkVertexInputBindingDescription mesh_dual_contouring_get_binding_description(void);
// internal inline void mesh_dual_contouring_get_attribute_descriptions(VkVertexInputAttributeDescription *attribute_descriptions);
//
// internal inline void mesh_manifold_dual_contouring_init(struct Mesh *mesh);
// internal inline void mesh_manifold_dual_contouring_assign_vertex(struct Vector *vector, r32 x, r32 y, r32 z, r32 r, r32 g, r32 b, r32 nr1, r32 ng1, r32 nb1, r32 nr2, r32 ng2, r32 nb2);
// internal inline void mesh_manifold_dual_contouring_assign_vertex_simple(struct Vector *vector, struct VertexManifoldDualContouring vertex);
// internal inline VkVertexInputBindingDescription mesh_manifold_dual_contouring_get_binding_description(void);
// internal inline void mesh_manifold_dual_contouring_get_attribute_descriptions(VkVertexInputAttributeDescription *attribute_descriptions);
//
// internal inline void mesh_grass_init(struct Mesh *mesh);
// internal inline void mesh_grass_assign_vertex(struct Vector *vector, r32 x, r32 y, r32 z, r32 w);
// internal inline VkVertexInputBindingDescription mesh_grass_get_binding_description(void);
// internal inline void mesh_grass_get_attribute_descriptions(VkVertexInputAttributeDescription *attribute_descriptions);
//
// internal inline void mesh_delete(struct Mesh *mesh);
// internal inline void mesh_clear(struct Mesh *mesh);
// internal inline void mesh_clear_vertices(struct Mesh *mesh);
// internal inline void mesh_clear_indices(struct Mesh *mesh);
// internal inline void mesh_assign_indice(struct Vector *vector, u32 indice);
//
///////////////////////////////////////////////////////////////////////////////////////
//
// internal inline void mesh_sprite_init(struct Mesh *mesh) {
//  mesh->vertices = calloc(1, sizeof(struct Vector));
//  vector_init(mesh->vertices, sizeof(struct VertexSprite));
//
//  mesh->indices = calloc(1, sizeof(struct Vector));
//  vector_init(mesh->indices, sizeof(u32));
//}
//
// internal inline void mesh_sprite_assign_vertex(struct Vector *vector, r32 x, r32 y, r32 z, r32 u, r32 v) {
//  struct VertexSprite vertex;
//  memset(&vertex, 0, sizeof(vertex));
//
//  vertex.position.x = x;
//  vertex.position.y = y;
//  vertex.position.z = z;
//
//  vertex.tex_coord.u = u;
//  vertex.tex_coord.v = v;
//
//  vector_push_back(vector, &vertex);
//}
//
// internal inline VkVertexInputBindingDescription mesh_sprite_get_binding_description(void) {
//  VkVertexInputBindingDescription binding_description = {0};
//  binding_description.binding = 0;
//  binding_description.stride = sizeof(struct VertexSprite);
//  binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
//
//  return binding_description;
//}
//
// internal inline void mesh_sprite_get_attribute_descriptions(VkVertexInputAttributeDescription *attribute_descriptions) {
//  attribute_descriptions[0].binding = 0;
//  attribute_descriptions[0].location = 0;
//  attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
//  attribute_descriptions[0].offset = offsetof(struct VertexSprite, position);
//
//  attribute_descriptions[1].binding = 0;
//  attribute_descriptions[1].location = 1;
//  attribute_descriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
//  attribute_descriptions[1].offset = offsetof(struct VertexSprite, tex_coord);
//}
//
///////////////////////////////////////////////////////////////////////////////////////
//
// internal inline void mesh_quad_init(struct Mesh *mesh) {
//  mesh->vertices = calloc(1, sizeof(struct Vector));
//  vector_init(mesh->vertices, sizeof(struct VertexQuad));
//
//  mesh->indices = calloc(1, sizeof(struct Vector));
//  vector_init(mesh->indices, sizeof(u32));
//
//  mesh_quad_assign_vertex(mesh->vertices, -0.5f, -0.5f, 0.0f);
//  mesh_quad_assign_vertex(mesh->vertices, 0.5f, -0.5f, 0.0f);
//  mesh_quad_assign_vertex(mesh->vertices, 0.5f, 0.5f, 0.0f);
//  mesh_quad_assign_vertex(mesh->vertices, -0.5f, 0.5f, 0.0f);
//
//  mesh_assign_indice(mesh->indices, 0);
//  mesh_assign_indice(mesh->indices, 1);
//  mesh_assign_indice(mesh->indices, 2);
//  mesh_assign_indice(mesh->indices, 2);
//  mesh_assign_indice(mesh->indices, 3);
//  mesh_assign_indice(mesh->indices, 0);
//}
//
// internal inline void mesh_quad_assign_vertex(struct Vector *vector, r32 x, r32 y, r32 z) {
//  struct VertexQuad vertex;
//  memset(&vertex, 0, sizeof(vertex));
//
//  vertex.position.x = x;
//  vertex.position.y = y;
//  vertex.position.z = z;
//
//  vector_push_back(vector, &vertex);
//}
//
// internal inline VkVertexInputBindingDescription mesh_quad_get_binding_description(void) {
//  VkVertexInputBindingDescription binding_description = {0};
//  binding_description.binding = 0;
//  binding_description.stride = sizeof(struct VertexQuad);
//  binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
//
//  return binding_description;
//}
//
// internal inline void mesh_quad_get_attribute_descriptions(VkVertexInputAttributeDescription *attribute_descriptions) {
//  attribute_descriptions[0].binding = 0;
//  attribute_descriptions[0].location = 0;
//  attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
//  attribute_descriptions[0].offset = offsetof(struct VertexQuad, position);
//}
//
///////////////////////////////////////////////////////////////////////////////////////
//
// internal inline void mesh_triangle_init(struct Mesh *mesh) {
//  mesh->vertices = calloc(1, sizeof(struct Vector));
//  vector_init(mesh->vertices, sizeof(struct VertexTriangle));
//
//  mesh->indices = calloc(1, sizeof(struct Vector));
//  vector_init(mesh->indices, sizeof(u32));
//
//  mesh_triangle_assign_vertex(mesh->vertices, -1.0f, 1.0f, 0.0f);
//  mesh_triangle_assign_vertex(mesh->vertices, -1.0f, -2.0f, 0.0f);
//  mesh_triangle_assign_vertex(mesh->vertices, 2.0f, 1.0f, 0.0f);
//
//  mesh_assign_indice(mesh->indices, 0);
//  mesh_assign_indice(mesh->indices, 1);
//  mesh_assign_indice(mesh->indices, 2);
//}
//
// internal inline void mesh_triangle_assign_vertex(struct Vector *vector, r32 x, r32 y, r32 z) {
//  struct VertexTriangle vertex;
//  memset(&vertex, 0, sizeof(vertex));
//
//  vertex.position.x = x;
//  vertex.position.y = y;
//  vertex.position.z = z;
//
//  vector_push_back(vector, &vertex);
//}
//
// internal inline VkVertexInputBindingDescription mesh_triangle_get_binding_description(void) {
//  VkVertexInputBindingDescription binding_description = {0};
//  binding_description.binding = 0;
//  binding_description.stride = sizeof(struct VertexTriangle);
//  binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
//
//  return binding_description;
//}
//
// internal inline void mesh_triangle_get_attribute_descriptions(VkVertexInputAttributeDescription *attribute_descriptions) {
//  attribute_descriptions[0].binding = 0;
//  attribute_descriptions[0].location = 0;
//  attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
//  attribute_descriptions[0].offset = offsetof(struct VertexTriangle, position);
//}
//
///////////////////////////////////////////////////////////////////////////////////////
//
// internal inline void mesh_model_init(struct Mesh *mesh) {
//  mesh->vertices = calloc(1, sizeof(struct Vector));
//  vector_init(mesh->vertices, sizeof(struct VertexModel));
//
//  mesh->indices = calloc(1, sizeof(struct Vector));
//  vector_init(mesh->indices, sizeof(u32));
//}
//
// internal inline void mesh_model_assign_vertex(struct Vector *vector, r32 x, r32 y, r32 z, r32 r1, r32 g1, r32 b1, r32 u, r32 v, r32 r2, r32 g2, r32 b2, int joint_id_x, int joint_id_y, int joint_id_z, r32 weight_x, r32 weight_y, r32 weight_z) {
//  struct VertexModel vertex;
//  memset(&vertex, 0, sizeof(vertex));
//
//  vertex.position.x = x;
//  vertex.position.y = y;
//  vertex.position.z = z;
//
//  vertex.normal.x = r1;
//  vertex.normal.y = g1;
//  vertex.normal.z = b1;
//
//  vertex.tex_coord.u = u;
//  vertex.tex_coord.v = v;
//
//  vertex.color.r = r2;
//  vertex.color.g = g2;
//  vertex.color.b = b2;
//
//  vertex.joints_ids.id0 = joint_id_x;
//  vertex.joints_ids.id1 = joint_id_y;
//  vertex.joints_ids.id2 = joint_id_z;
//
//  vertex.weights.data[0] = weight_x;
//  vertex.weights.data[1] = weight_y;
//  vertex.weights.data[2] = weight_z;
//
//  vector_push_back(vector, &vertex);
//}
//
// internal inline VkVertexInputBindingDescription mesh_model_get_binding_description(void) {
//  VkVertexInputBindingDescription binding_description = {0};
//  binding_description.binding = 0;
//  binding_description.stride = sizeof(struct VertexModel);
//  binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
//
//  return binding_description;
//}
//
// internal inline void mesh_model_get_attribute_descriptions(VkVertexInputAttributeDescription *attribute_descriptions) {
//  attribute_descriptions[0].binding = 0;
//  attribute_descriptions[0].location = 0;
//  attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
//  attribute_descriptions[0].offset = offsetof(struct VertexModel, position);
//
//  attribute_descriptions[1].binding = 0;
//  attribute_descriptions[1].location = 1;
//  attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
//  attribute_descriptions[1].offset = offsetof(struct VertexModel, normal);
//
//  attribute_descriptions[2].binding = 0;
//  attribute_descriptions[2].location = 2;
//  attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
//  attribute_descriptions[2].offset = offsetof(struct VertexModel, tex_coord);
//
//  attribute_descriptions[3].binding = 0;
//  attribute_descriptions[3].location = 3;
//  attribute_descriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
//  attribute_descriptions[3].offset = offsetof(struct VertexModel, color);
//
//  attribute_descriptions[4].binding = 0;
//  attribute_descriptions[4].location = 4;
//  attribute_descriptions[4].format = VK_FORMAT_R32G32B32_SINT;
//  attribute_descriptions[4].offset = offsetof(struct VertexModel, joints_ids);
//
//  attribute_descriptions[5].binding = 0;
//  attribute_descriptions[5].location = 5;
//  attribute_descriptions[5].format = VK_FORMAT_R32G32B32_SFLOAT;
//  attribute_descriptions[5].offset = offsetof(struct VertexModel, weights);
//}
//
///////////////////////////////////////////////////////////////////////////////////////
//
// internal inline void mesh_model_internal_init(struct Mesh *mesh) {
//  mesh->vertices = calloc(1, sizeof(struct Vector));
//  vector_init(mesh->vertices, sizeof(struct VertexModelinternal));
//
//  mesh->indices = calloc(1, sizeof(struct Vector));
//  vector_init(mesh->indices, sizeof(u32));
//}
//
// internal inline void mesh_model_internal_assign_vertex(struct Vector *vector, r32 x, r32 y, r32 z, r32 r1, r32 g1, r32 b1, r32 u, r32 v, r32 r2, r32 g2, r32 b2) {
//  struct VertexModelinternal vertex;
//  memset(&vertex, 0, sizeof(vertex));
//
//  vertex.position.x = x;
//  vertex.position.y = y;
//  vertex.position.z = z;
//
//  vertex.normal.x = r1;
//  vertex.normal.y = g1;
//  vertex.normal.z = b1;
//
//  vertex.tex_coord.u = u;
//  vertex.tex_coord.v = v;
//
//  vertex.color.r = r2;
//  vertex.color.g = g2;
//  vertex.color.b = b2;
//
//  vector_push_back(vector, &vertex);
//}
//
// internal inline VkVertexInputBindingDescription mesh_model_internal_get_binding_description(void) {
//  VkVertexInputBindingDescription binding_description = {0};
//  binding_description.binding = 0;
//  binding_description.stride = sizeof(struct VertexModelinternal);
//  binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
//
//  return binding_description;
//}
//
// internal inline void mesh_model_internal_get_attribute_descriptions(VkVertexInputAttributeDescription *attribute_descriptions) {
//  attribute_descriptions[0].binding = 0;
//  attribute_descriptions[0].location = 0;
//  attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
//  attribute_descriptions[0].offset = offsetof(struct VertexModelinternal, position);
//
//  attribute_descriptions[1].binding = 0;
//  attribute_descriptions[1].location = 1;
//  attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
//  attribute_descriptions[1].offset = offsetof(struct VertexModelinternal, normal);
//
//  attribute_descriptions[2].binding = 0;
//  attribute_descriptions[2].location = 2;
//  attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
//  attribute_descriptions[2].offset = offsetof(struct VertexModelinternal, tex_coord);
//
//  attribute_descriptions[3].binding = 0;
//  attribute_descriptions[3].location = 3;
//  attribute_descriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
//  attribute_descriptions[3].offset = offsetof(struct VertexModelinternal, color);
//}
//
///////////////////////////////////////////////////////////////////////////////////////
//
// internal inline void mesh_dual_contouring_init(struct Mesh *mesh) {
//  mesh->vertices = calloc(1, sizeof(struct Vector));
//  vector_init(mesh->vertices, sizeof(struct VertexDualContouring));
//
//  mesh->indices = calloc(1, sizeof(struct Vector));
//  vector_init(mesh->indices, sizeof(u32));
//}
//
// internal inline void mesh_dual_contouring_assign_vertex(struct Vector *vector, r32 x, r32 y, r32 z, r32 r, r32 g, r32 b) {
//  struct VertexDualContouring vertex;
//  memset(&vertex, 0, sizeof(vertex));
//
//  vertex.position.x = x;
//  vertex.position.y = y;
//  vertex.position.z = z;
//
//  vertex.normal.x = r;
//  vertex.normal.y = g;
//  vertex.normal.z = b;
//
//  vector_push_back(vector, &vertex);
//}
//
// internal inline VkVertexInputBindingDescription mesh_dual_contouring_get_binding_description(void) {
//  VkVertexInputBindingDescription binding_description = {0};
//  binding_description.binding = 0;
//  binding_description.stride = sizeof(struct VertexDualContouring);
//  binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
//
//  return binding_description;
//}
//
// internal inline void mesh_dual_contouring_get_attribute_descriptions(VkVertexInputAttributeDescription *attribute_descriptions) {
//  attribute_descriptions[0].binding = 0;
//  attribute_descriptions[0].location = 0;
//  attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
//  attribute_descriptions[0].offset = offsetof(struct VertexDualContouring, position);
//
//  attribute_descriptions[1].binding = 0;
//  attribute_descriptions[1].location = 1;
//  attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
//  attribute_descriptions[1].offset = offsetof(struct VertexDualContouring, normal);
//}
//
///////////////////////////////////////////////////////////////////////////////////////
//
// internal inline void mesh_manifold_dual_contouring_init(struct Mesh *mesh) {
//  mesh->vertices = calloc(1, sizeof(struct Vector));
//  vector_init(mesh->vertices, sizeof(struct VertexManifoldDualContouring));
//
//  mesh->indices = calloc(1, sizeof(struct Vector));
//  vector_init(mesh->indices, sizeof(u32));
//}
//
// internal inline void mesh_manifold_dual_contouring_assign_vertex(struct Vector *vector, r32 x, r32 y, r32 z, r32 r, r32 g, r32 b, r32 nr1, r32 ng1, r32 nb1, r32 nr2, r32 ng2, r32 nb2) {
//  struct VertexManifoldDualContouring vertex;
//  memset(&vertex, 0, sizeof(vertex));
//
//  vertex.position.x = x;
//  vertex.position.y = y;
//  vertex.position.z = z;
//
//  vertex.color.r = r;
//  vertex.color.g = g;
//  vertex.color.b = b;
//
//  vertex.normal1.r = nr1;
//  vertex.normal1.g = ng1;
//  vertex.normal1.b = nb1;
//
//  vertex.normal2.r = nr2;
//  vertex.normal2.g = ng2;
//  vertex.normal2.b = nb2;
//
//  vector_push_back(vector, &vertex);
//}
//
// internal inline void mesh_manifold_dual_contouring_assign_vertex_simple(struct Vector *vector, struct VertexManifoldDualContouring vertex) {
//  vector_push_back(vector, &vertex);
//}
//
// internal inline VkVertexInputBindingDescription mesh_manifold_dual_contouring_get_binding_description(void) {
//  VkVertexInputBindingDescription binding_description = {0};
//  binding_description.binding = 0;
//  binding_description.stride = sizeof(struct VertexManifoldDualContouring);
//  binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
//
//  return binding_description;
//}
//
// internal inline void mesh_manifold_dual_contouring_get_attribute_descriptions(VkVertexInputAttributeDescription *attribute_descriptions) {
//  attribute_descriptions[0].binding = 0;
//  attribute_descriptions[0].location = 0;
//  attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
//  attribute_descriptions[0].offset = offsetof(struct VertexManifoldDualContouring, position);
//
//  attribute_descriptions[1].binding = 0;
//  attribute_descriptions[1].location = 1;
//  attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
//  attribute_descriptions[1].offset = offsetof(struct VertexManifoldDualContouring, color);
//
//  attribute_descriptions[2].binding = 0;
//  attribute_descriptions[2].location = 2;
//  attribute_descriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
//  attribute_descriptions[2].offset = offsetof(struct VertexManifoldDualContouring, normal1);
//
//  attribute_descriptions[3].binding = 0;
//  attribute_descriptions[3].location = 3;
//  attribute_descriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
//  attribute_descriptions[3].offset = offsetof(struct VertexManifoldDualContouring, normal2);
//}
//
///////////////////////////////////////////////////////////////////////////////////////
//
// internal inline void mesh_grass_init(struct Mesh *mesh) {
//  mesh->vertices = calloc(1, sizeof(struct Vector));
//  vector_init(mesh->vertices, sizeof(struct VertexGrass));
//
//  mesh->indices = calloc(1, sizeof(struct Vector));
//  vector_init(mesh->indices, sizeof(u32));
//}
//
// internal inline void mesh_grass_assign_vertex(struct Vector *vector, r32 x, r32 y, r32 z, r32 w) {
//  struct VertexGrass vertex;
//  memset(&vertex, 0, sizeof(vertex));
//
//  vertex.position_color.x = x;
//  vertex.position_color.y = y;
//  vertex.position_color.z = z;
//  vertex.position_color.w = w;
//
//  vector_push_back(vector, &vertex);
//}
//
// internal inline VkVertexInputBindingDescription mesh_grass_get_binding_description(void) {
//  VkVertexInputBindingDescription binding_description = {0};
//  binding_description.binding = 0;
//  binding_description.stride = sizeof(struct VertexGrass);
//  binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
//
//  return binding_description;
//}
//
// internal inline void mesh_grass_get_attribute_descriptions(VkVertexInputAttributeDescription *attribute_descriptions) {
//  attribute_descriptions[0].binding = 0;
//  attribute_descriptions[0].location = 0;
//  attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
//  attribute_descriptions[0].offset = offsetof(struct VertexGrass, position_color);
//}
//
///////////////////////////////////////////////////////////////////////////////////////
