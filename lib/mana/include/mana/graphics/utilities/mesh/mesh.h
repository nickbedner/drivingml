#pragma once

#include "mana/graphics/apis/api.h"
#ifdef VULKAN_API_SUPPORTED
#include "mana/graphics/utilities/mesh/meshvulkan.h"
#endif
#ifdef DIRECTX_12_API_SUPPORTED
#include "mana/graphics/utilities/mesh/meshdirectx12.h"
#endif

struct MeshFunc {
  uint_fast8_t (*mesh_init)(struct MeshCommon *, enum MESH_TYPE, struct APICommon *);
  void (*mesh_delete)(struct MeshCommon *, struct APICommon *);
  void (*mesh_generate_buffers)(struct MeshCommon *, struct APICommon *);
};

#ifdef VULKAN_API_SUPPORTED
static const struct MeshFunc MESH_VULKAN = {mesh_vulkan_init, mesh_vulkan_delete, mesh_vulkan_generate_buffers};
#endif
#ifdef DIRECTX_12_API_SUPPORTED
static const struct MeshFunc MESH_DIRECTX12 = {mesh_directx_12_init, mesh_directx_12_delete, mesh_directx_12_generate_buffers};
#endif

struct Mesh {
  struct MeshFunc mesh_func;
  struct MeshCommon mesh_common;
};

uint_fast8_t mesh_init(struct Mesh *mesh, enum MESH_TYPE mesh_type, struct APICommon *api_common);
void mesh_delete(struct Mesh *mesh, struct APICommon *api_common);
uint32_t mesh_get_memory_size(enum MESH_TYPE mesh_type);
void mesh_generate_buffers(struct Mesh *mesh, struct APICommon *api_common);
void mesh_clear(struct Mesh *mesh);
void mesh_clear_vertices(struct Mesh *mesh);
void mesh_clear_indices(struct Mesh *mesh);
void mesh_assign_vertex(struct Mesh *mesh, void *vertex);
void mesh_assign_indice(struct Mesh *mesh, uint32_t indice);

void mesh_fullscreen_triangle(struct Mesh *mesh);

// static inline void mesh_sprite_init(struct Mesh *mesh);
// static inline void mesh_sprite_assign_vertex(struct Vector *vector, float x, float y, float z, float u, float v);
// static inline VkVertexInputBindingDescription mesh_sprite_get_binding_description(void);
// static inline void mesh_sprite_get_attribute_descriptions(VkVertexInputAttributeDescription *attribute_descriptions);
//
// static inline void mesh_quad_init(struct Mesh *mesh);
// static inline void mesh_quad_assign_vertex(struct Vector *vector, float x, float y, float z);
// static inline VkVertexInputBindingDescription mesh_quad_get_binding_description(void);
// static inline void mesh_quad_get_attribute_descriptions(VkVertexInputAttributeDescription *attribute_descriptions);
//
// static inline void mesh_triangle_init(struct Mesh *mesh);
// static inline void mesh_triangle_assign_vertex(struct Vector *vector, float x, float y, float z);
// static inline VkVertexInputBindingDescription mesh_triangle_get_binding_description(void);
// static inline void mesh_triangle_get_attribute_descriptions(VkVertexInputAttributeDescription *attribute_descriptions);
//
// static inline void mesh_model_init(struct Mesh *mesh);
// static inline void mesh_model_assign_vertex(struct Vector *vector, float x, float y, float z, float r1, float g1, float b1, float u, float v, float r2, float g2, float b2, int joint_id_x, int joint_id_y, int joint_id_z, float weight_x, float weight_y, float weight_z);
// static inline VkVertexInputBindingDescription mesh_model_get_binding_description(void);
// static inline void mesh_model_get_attribute_descriptions(VkVertexInputAttributeDescription *attribute_descriptions);
//
// static inline void mesh_model_static_init(struct Mesh *mesh);
// static inline void mesh_model_static_assign_vertex(struct Vector *vector, float x, float y, float z, float r1, float g1, float b1, float u, float v, float r2, float g2, float b2);
// static inline VkVertexInputBindingDescription mesh_model_static_get_binding_description(void);
// static inline void mesh_model_static_get_attribute_descriptions(VkVertexInputAttributeDescription *attribute_descriptions);
//
// static inline void mesh_dual_contouring_init(struct Mesh *mesh);
// static inline void mesh_dual_contouring_assign_vertex(struct Vector *vector, float x, float y, float z, float r, float g, float b);
// static inline VkVertexInputBindingDescription mesh_dual_contouring_get_binding_description(void);
// static inline void mesh_dual_contouring_get_attribute_descriptions(VkVertexInputAttributeDescription *attribute_descriptions);
//
// static inline void mesh_manifold_dual_contouring_init(struct Mesh *mesh);
// static inline void mesh_manifold_dual_contouring_assign_vertex(struct Vector *vector, float x, float y, float z, float r, float g, float b, float nr1, float ng1, float nb1, float nr2, float ng2, float nb2);
// static inline void mesh_manifold_dual_contouring_assign_vertex_simple(struct Vector *vector, struct VertexManifoldDualContouring vertex);
// static inline VkVertexInputBindingDescription mesh_manifold_dual_contouring_get_binding_description(void);
// static inline void mesh_manifold_dual_contouring_get_attribute_descriptions(VkVertexInputAttributeDescription *attribute_descriptions);
//
// static inline void mesh_grass_init(struct Mesh *mesh);
// static inline void mesh_grass_assign_vertex(struct Vector *vector, float x, float y, float z, float w);
// static inline VkVertexInputBindingDescription mesh_grass_get_binding_description(void);
// static inline void mesh_grass_get_attribute_descriptions(VkVertexInputAttributeDescription *attribute_descriptions);
//
// static inline void mesh_delete(struct Mesh *mesh);
// static inline void mesh_clear(struct Mesh *mesh);
// static inline void mesh_clear_vertices(struct Mesh *mesh);
// static inline void mesh_clear_indices(struct Mesh *mesh);
// static inline void mesh_assign_indice(struct Vector *vector, uint32_t indice);
//
///////////////////////////////////////////////////////////////////////////////////////
//
// static inline void mesh_sprite_init(struct Mesh *mesh) {
//  mesh->vertices = calloc(1, sizeof(struct Vector));
//  vector_init(mesh->vertices, sizeof(struct VertexSprite));
//
//  mesh->indices = calloc(1, sizeof(struct Vector));
//  vector_init(mesh->indices, sizeof(uint32_t));
//}
//
// static inline void mesh_sprite_assign_vertex(struct Vector *vector, float x, float y, float z, float u, float v) {
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
// static inline VkVertexInputBindingDescription mesh_sprite_get_binding_description(void) {
//  VkVertexInputBindingDescription binding_description = {0};
//  binding_description.binding = 0;
//  binding_description.stride = sizeof(struct VertexSprite);
//  binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
//
//  return binding_description;
//}
//
// static inline void mesh_sprite_get_attribute_descriptions(VkVertexInputAttributeDescription *attribute_descriptions) {
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
// static inline void mesh_quad_init(struct Mesh *mesh) {
//  mesh->vertices = calloc(1, sizeof(struct Vector));
//  vector_init(mesh->vertices, sizeof(struct VertexQuad));
//
//  mesh->indices = calloc(1, sizeof(struct Vector));
//  vector_init(mesh->indices, sizeof(uint32_t));
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
// static inline void mesh_quad_assign_vertex(struct Vector *vector, float x, float y, float z) {
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
// static inline VkVertexInputBindingDescription mesh_quad_get_binding_description(void) {
//  VkVertexInputBindingDescription binding_description = {0};
//  binding_description.binding = 0;
//  binding_description.stride = sizeof(struct VertexQuad);
//  binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
//
//  return binding_description;
//}
//
// static inline void mesh_quad_get_attribute_descriptions(VkVertexInputAttributeDescription *attribute_descriptions) {
//  attribute_descriptions[0].binding = 0;
//  attribute_descriptions[0].location = 0;
//  attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
//  attribute_descriptions[0].offset = offsetof(struct VertexQuad, position);
//}
//
///////////////////////////////////////////////////////////////////////////////////////
//
// static inline void mesh_triangle_init(struct Mesh *mesh) {
//  mesh->vertices = calloc(1, sizeof(struct Vector));
//  vector_init(mesh->vertices, sizeof(struct VertexTriangle));
//
//  mesh->indices = calloc(1, sizeof(struct Vector));
//  vector_init(mesh->indices, sizeof(uint32_t));
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
// static inline void mesh_triangle_assign_vertex(struct Vector *vector, float x, float y, float z) {
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
// static inline VkVertexInputBindingDescription mesh_triangle_get_binding_description(void) {
//  VkVertexInputBindingDescription binding_description = {0};
//  binding_description.binding = 0;
//  binding_description.stride = sizeof(struct VertexTriangle);
//  binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
//
//  return binding_description;
//}
//
// static inline void mesh_triangle_get_attribute_descriptions(VkVertexInputAttributeDescription *attribute_descriptions) {
//  attribute_descriptions[0].binding = 0;
//  attribute_descriptions[0].location = 0;
//  attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
//  attribute_descriptions[0].offset = offsetof(struct VertexTriangle, position);
//}
//
///////////////////////////////////////////////////////////////////////////////////////
//
// static inline void mesh_model_init(struct Mesh *mesh) {
//  mesh->vertices = calloc(1, sizeof(struct Vector));
//  vector_init(mesh->vertices, sizeof(struct VertexModel));
//
//  mesh->indices = calloc(1, sizeof(struct Vector));
//  vector_init(mesh->indices, sizeof(uint32_t));
//}
//
// static inline void mesh_model_assign_vertex(struct Vector *vector, float x, float y, float z, float r1, float g1, float b1, float u, float v, float r2, float g2, float b2, int joint_id_x, int joint_id_y, int joint_id_z, float weight_x, float weight_y, float weight_z) {
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
// static inline VkVertexInputBindingDescription mesh_model_get_binding_description(void) {
//  VkVertexInputBindingDescription binding_description = {0};
//  binding_description.binding = 0;
//  binding_description.stride = sizeof(struct VertexModel);
//  binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
//
//  return binding_description;
//}
//
// static inline void mesh_model_get_attribute_descriptions(VkVertexInputAttributeDescription *attribute_descriptions) {
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
// static inline void mesh_model_static_init(struct Mesh *mesh) {
//  mesh->vertices = calloc(1, sizeof(struct Vector));
//  vector_init(mesh->vertices, sizeof(struct VertexModelStatic));
//
//  mesh->indices = calloc(1, sizeof(struct Vector));
//  vector_init(mesh->indices, sizeof(uint32_t));
//}
//
// static inline void mesh_model_static_assign_vertex(struct Vector *vector, float x, float y, float z, float r1, float g1, float b1, float u, float v, float r2, float g2, float b2) {
//  struct VertexModelStatic vertex;
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
// static inline VkVertexInputBindingDescription mesh_model_static_get_binding_description(void) {
//  VkVertexInputBindingDescription binding_description = {0};
//  binding_description.binding = 0;
//  binding_description.stride = sizeof(struct VertexModelStatic);
//  binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
//
//  return binding_description;
//}
//
// static inline void mesh_model_static_get_attribute_descriptions(VkVertexInputAttributeDescription *attribute_descriptions) {
//  attribute_descriptions[0].binding = 0;
//  attribute_descriptions[0].location = 0;
//  attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
//  attribute_descriptions[0].offset = offsetof(struct VertexModelStatic, position);
//
//  attribute_descriptions[1].binding = 0;
//  attribute_descriptions[1].location = 1;
//  attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
//  attribute_descriptions[1].offset = offsetof(struct VertexModelStatic, normal);
//
//  attribute_descriptions[2].binding = 0;
//  attribute_descriptions[2].location = 2;
//  attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
//  attribute_descriptions[2].offset = offsetof(struct VertexModelStatic, tex_coord);
//
//  attribute_descriptions[3].binding = 0;
//  attribute_descriptions[3].location = 3;
//  attribute_descriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
//  attribute_descriptions[3].offset = offsetof(struct VertexModelStatic, color);
//}
//
///////////////////////////////////////////////////////////////////////////////////////
//
// static inline void mesh_dual_contouring_init(struct Mesh *mesh) {
//  mesh->vertices = calloc(1, sizeof(struct Vector));
//  vector_init(mesh->vertices, sizeof(struct VertexDualContouring));
//
//  mesh->indices = calloc(1, sizeof(struct Vector));
//  vector_init(mesh->indices, sizeof(uint32_t));
//}
//
// static inline void mesh_dual_contouring_assign_vertex(struct Vector *vector, float x, float y, float z, float r, float g, float b) {
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
// static inline VkVertexInputBindingDescription mesh_dual_contouring_get_binding_description(void) {
//  VkVertexInputBindingDescription binding_description = {0};
//  binding_description.binding = 0;
//  binding_description.stride = sizeof(struct VertexDualContouring);
//  binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
//
//  return binding_description;
//}
//
// static inline void mesh_dual_contouring_get_attribute_descriptions(VkVertexInputAttributeDescription *attribute_descriptions) {
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
// static inline void mesh_manifold_dual_contouring_init(struct Mesh *mesh) {
//  mesh->vertices = calloc(1, sizeof(struct Vector));
//  vector_init(mesh->vertices, sizeof(struct VertexManifoldDualContouring));
//
//  mesh->indices = calloc(1, sizeof(struct Vector));
//  vector_init(mesh->indices, sizeof(uint32_t));
//}
//
// static inline void mesh_manifold_dual_contouring_assign_vertex(struct Vector *vector, float x, float y, float z, float r, float g, float b, float nr1, float ng1, float nb1, float nr2, float ng2, float nb2) {
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
// static inline void mesh_manifold_dual_contouring_assign_vertex_simple(struct Vector *vector, struct VertexManifoldDualContouring vertex) {
//  vector_push_back(vector, &vertex);
//}
//
// static inline VkVertexInputBindingDescription mesh_manifold_dual_contouring_get_binding_description(void) {
//  VkVertexInputBindingDescription binding_description = {0};
//  binding_description.binding = 0;
//  binding_description.stride = sizeof(struct VertexManifoldDualContouring);
//  binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
//
//  return binding_description;
//}
//
// static inline void mesh_manifold_dual_contouring_get_attribute_descriptions(VkVertexInputAttributeDescription *attribute_descriptions) {
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
// static inline void mesh_grass_init(struct Mesh *mesh) {
//  mesh->vertices = calloc(1, sizeof(struct Vector));
//  vector_init(mesh->vertices, sizeof(struct VertexGrass));
//
//  mesh->indices = calloc(1, sizeof(struct Vector));
//  vector_init(mesh->indices, sizeof(uint32_t));
//}
//
// static inline void mesh_grass_assign_vertex(struct Vector *vector, float x, float y, float z, float w) {
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
// static inline VkVertexInputBindingDescription mesh_grass_get_binding_description(void) {
//  VkVertexInputBindingDescription binding_description = {0};
//  binding_description.binding = 0;
//  binding_description.stride = sizeof(struct VertexGrass);
//  binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
//
//  return binding_description;
//}
//
// static inline void mesh_grass_get_attribute_descriptions(VkVertexInputAttributeDescription *attribute_descriptions) {
//  attribute_descriptions[0].binding = 0;
//  attribute_descriptions[0].location = 0;
//  attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
//  attribute_descriptions[0].offset = offsetof(struct VertexGrass, position_color);
//}
//
///////////////////////////////////////////////////////////////////////////////////////
