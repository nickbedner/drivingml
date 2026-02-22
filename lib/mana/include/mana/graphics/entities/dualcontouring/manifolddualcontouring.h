#pragma once

// #include <mana/math/advmath.h>
// #include <mana/noise/noise.h>
// #include <mana/storage/storage.h>
// #include <math.h>
//
// #include "mana/graphics/dualcontouring/manifoldtables.h"
// #include "mana/graphics/dualcontouring/qef.h"
// #include "mana/graphics/utilities/mesh.h"

enum NodeType {
  MANIFOLD_NODE_NONE,
  MANIFOLD_NODE_INTERNAL,
  MANIFOLD_NODE_LEAF,
  MANIFOLD_NODE_COLLAPSED
};

/*struct Vertex {
  int index;
  int in_cell;
  int surface_index;
  int euler;
  int eis[12];
  vec3 normal;
  int* vertice_reduction;
  struct Vertex* parent;
  bool collapsible;
  bool face_prop2;
  struct QefSolver qef;
  float error;
};

static inline void vertex_init(struct Vertex* vertex) {
  vertex->parent = NULL;
  vertex->index = -1;
  vertex->collapsible = true;
  qef_solver_init(&vertex->qef);
  vertex->normal = VEC3_ZERO;
  vertex->surface_index = -1;
  vertex->error = 0.0f;
  vertex->euler = 0;
  memset(vertex->eis, 0, sizeof(int) * 12);
  vertex->face_prop2 = false;
}

struct ManifoldOctreeNode {
  int index;
  int child_index;
  int size;
  int scale;
  ivec3 position;
  enum NodeType type;
  int children[8];
  int vertices[16];
  char total_vertices;
  char corners;
};

static inline void octree_node_init(struct ManifoldOctreeNode* octree_node, ivec3 position, int size, int scale, enum NodeType type) {
  octree_node->index = 0;
  octree_node->position = position;
  octree_node->size = size;
  octree_node->scale = scale;
  octree_node->type = type;
}

// TODO: Calculating normals can be much faster in a special noise function either by a single 8x1 array for SIMD with each position unique in 3D space
// OR calculate bulk in chunky 8*8 kernel sorta thing
static inline vec3 planet_normal(vec3 v, struct NoiseModule* planet_shape, int scale) {
  float h = 0.001f * scale;
  float dxp = noise_module_eval(planet_shape, v.x + h, v.y, v.z);
  float dxm = noise_module_eval(planet_shape, v.x - h, v.y, v.z);
  float dyp = noise_module_eval(planet_shape, v.x, v.y + h, v.z);
  float dym = noise_module_eval(planet_shape, v.x, v.y - h, v.z);
  float dzp = noise_module_eval(planet_shape, v.x, v.y, v.z + h);
  float dzm = noise_module_eval(planet_shape, v.x, v.y, v.z - h);
  vec3 gradient = (vec3){.x = dxp - dxm, .y = dyp - dym, .z = dzp - dzm};
  gradient = vec3_old_skool_normalise(gradient);
  return gradient;
}

void manifold_octree_init(struct ManifoldOctreeNode* octree_node, int resolution, int scale, struct ManifoldOctreeNode* node_cache, struct Vector* vertice_pool, struct NoiseModule* planet_shape);
void manifold_octree_delete(struct ManifoldOctreeNode* octree_node);
void manifold_octree_generate_vertex_buffer(struct ManifoldOctreeNode* octree_node, struct Vector* vertices, struct Vector* vertice_pool);
void manifold_octree_process_cell(struct ManifoldOctreeNode* octree_node, struct Vector* indexes, float threshold, struct Vector* vertice_pool);
void manifold_octree_cluster_cell_base(struct ManifoldOctreeNode* octree_node, float error, struct Vector* vertice_pool, struct NoiseModule* planet_shape);

struct ManifoldDualContouringUniformBufferObject {
  alignas(32) mat4 model;
  alignas(32) mat4 view;
  alignas(32) mat4 proj;
  alignas(32) vec3 camera_pos;
};

struct ManifoldDualContouring {
  struct ManifoldOctreeNode* tree;

  // TODO: These will need to be made into custom data structures. Start with this then add stack with preallocated pool
  struct Vector vertice_pool;
  struct Vector node_pool;

  // TODO: All these should be on the stack, well not pointers at least
  struct Shader* shader;
  struct Mesh* mesh;

  VkBuffer vertex_buffer;
  VkDeviceMemory vertex_buffer_memory;

  VkBuffer index_buffer;
  VkDeviceMemory index_buffer_memory;

  VkBuffer dc_uniform_buffer;
  VkDeviceMemory dc_uniform_buffer_memory;

  VkBuffer lighting_uniform_buffer;
  VkDeviceMemory lighting_uniform_buffer_memory;

  VkDescriptorSet descriptor_set;
};

void manifold_dual_contouring_init(struct ManifoldDualContouring* manifold_dual_contouring, struct APICommon* api_common, struct Shader* shader, float size);
void manifold_dual_contouring_delete(struct ManifoldDualContouring* manifold_dual_contouring, struct APICommon* api_common);
void manifold_dual_contouring_recreate(struct ManifoldDualContouring* manifold_dual_contouring, struct APICommon* api_common);
void manifold_dual_contouring_update(struct ManifoldDualContouring* manifold_dual_contouring);
void manifold_dual_contouring_contour(struct ManifoldDualContouring* manifold_dual_contouring, struct APICommon* api_common, struct NoiseModule* planet_shape, float threshold);
void manifold_dual_contouring_construct_tree_grid(struct ManifoldOctreeNode* node);
vec3 manifold_dual_contouring_get_normal_q(struct Vector* verts, int indexes[6], int index_length);
*/
