#include "mana/graphics/entities/dualcontouring/manifolddualcontouring.h"

/*void manifold_dual_contouring_init(struct ManifoldDualContouring* manifold_dual_contouring, struct APICommon* api_common, struct Shader* shader, r32 size) {
  manifold_dual_contouring->shader = shader;

  manifold_dual_contouring->mesh = calloc(1, sizeof(struct Mesh));
  mesh_manifold_dual_contouring_init(manifold_dual_contouring->mesh);

  vector_init(&manifold_dual_contouring->vertice_pool, sizeof(struct Vertex));
  vector_init(&manifold_dual_contouring->node_pool, sizeof(struct ManifoldOctreeNode));
}

internal inline void manifold_dual_contouring_vulkan_cleanup(struct ManifoldDualContouring* manifold_dual_contouring, struct APICommon* api_common) {
  vkDestroyBuffer(api_common->vulkan_api->device, manifold_dual_contouring->index_buffer, NULL);
  vkFreeMemory(api_common->vulkan_api->device, manifold_dual_contouring->index_buffer_memory, NULL);

  vkDestroyBuffer(api_common->vulkan_api->device, manifold_dual_contouring->vertex_buffer, NULL);
  vkFreeMemory(api_common->vulkan_api->device, manifold_dual_contouring->vertex_buffer_memory, NULL);

  vkDestroyBuffer(api_common->vulkan_api->device, manifold_dual_contouring->lighting_uniform_buffer, NULL);
  vkFreeMemory(api_common->vulkan_api->device, manifold_dual_contouring->lighting_uniform_buffer_memory, NULL);

  vkDestroyBuffer(api_common->vulkan_api->device, manifold_dual_contouring->dc_uniform_buffer, NULL);
  vkFreeMemory(api_common->vulkan_api->device, manifold_dual_contouring->dc_uniform_buffer_memory, NULL);
}

void manifold_dual_contouring_delete(struct ManifoldDualContouring* manifold_dual_contouring, struct APICommon* api_common) {
  manifold_dual_contouring_vulkan_cleanup(manifold_dual_contouring, api_common);

  manifold_octree_delete(manifold_dual_contouring->tree);

  vector_delete(&manifold_dual_contouring->vertice_pool);
  vector_delete(&manifold_dual_contouring->node_pool);

  mesh_delete(manifold_dual_contouring->mesh);
  free(manifold_dual_contouring->mesh);

  free(manifold_dual_contouring->tree);
}

internal inline void manifold_dual_contouring_setup_buffers(struct ManifoldDualContouring* manifold_dual_contouring, struct APICommon* api_common) {
  graphics_utils_setup_vertex_buffer(api_common->vulkan_api, manifold_dual_contouring->mesh->vertices, &manifold_dual_contouring->vertex_buffer, &manifold_dual_contouring->vertex_buffer_memory);
  graphics_utils_setup_index_buffer(api_common->vulkan_api, manifold_dual_contouring->mesh->indices, &manifold_dual_contouring->index_buffer, &manifold_dual_contouring->index_buffer_memory);
  graphics_utils_setup_uniform_buffer(api_common->vulkan_api, sizeof(struct ManifoldDualContouringUniformBufferObject), &manifold_dual_contouring->dc_uniform_buffer, &manifold_dual_contouring->dc_uniform_buffer_memory);
  graphics_utils_setup_uniform_buffer(api_common->vulkan_api, sizeof(struct LightingUniformBufferObject), &manifold_dual_contouring->lighting_uniform_buffer, &manifold_dual_contouring->lighting_uniform_buffer_memory);
  graphics_utils_setup_descriptor(api_common->vulkan_api, manifold_dual_contouring->shader->descriptor_set_layout, manifold_dual_contouring->shader->descriptor_pool, &manifold_dual_contouring->descriptor_set);

  VkWriteDescriptorSet dcs[2] = {0};
  graphics_utils_setup_descriptor_buffer(dcs, 0, &manifold_dual_contouring->descriptor_set, (VkDescriptorBufferInfo[]){graphics_utils_setup_descriptor_buffer_info(sizeof(struct ManifoldDualContouringUniformBufferObject), &manifold_dual_contouring->dc_uniform_buffer)});
  graphics_utils_setup_descriptor_buffer(dcs, 1, &manifold_dual_contouring->descriptor_set, (VkDescriptorBufferInfo[]){graphics_utils_setup_descriptor_buffer_info(sizeof(struct LightingUniformBufferObject), &manifold_dual_contouring->lighting_uniform_buffer)});

  vkUpdateDescriptorSets(api_common->vulkan_api->device, 2, dcs, 0, NULL);
}

void manifold_dual_contouring_recreate(struct ManifoldDualContouring* manifold_dual_contouring, struct APICommon* api_common) {
  manifold_dual_contouring_vulkan_cleanup(manifold_dual_contouring, api_common);
  manifold_dual_contouring_setup_buffers(manifold_dual_contouring, api_common);
}

void manifold_dual_contouring_update(struct ManifoldDualContouring* manifold_dual_contouring) {
}

void manifold_dual_contouring_contour(struct ManifoldDualContouring* manifold_dual_contouring, struct APICommon* api_common, struct NoiseModule* planet_shape, r32 threshold) {
  // TODO: Move this to init?
  mesh_clear(manifold_dual_contouring->mesh);
  manifold_dual_contouring->tree = calloc(1, sizeof(struct ManifoldOctreeNode));
  manifold_dual_contouring->tree->index = 0;
  manifold_dual_contouring->tree->position = IVEC3_ZERO;
  manifold_dual_contouring->tree->type = MANIFOLD_NODE_INTERNAL;
  manifold_dual_contouring->tree->child_index = 0;

  // r64 start_time, end_time;
  // start_time = engine_get_time();
  // end_time = engine_get_time();
  // printf("Construct base time taken: %lf\n", end_time - start_time);
  manifold_octree_init(manifold_dual_contouring->tree, manifold_dual_contouring->resolution, pow(2, manifold_dual_contouring->max_subdivisions), manifold_dual_contouring->vertice_pool, planet_shape);
  manifold_octree_cluster_cell_base(manifold_dual_contouring->tree, 0, manifold_dual_contouring->vertice_pool, planet_shape);                                // Finds vertices
  manifold_octree_generate_vertex_buffer(manifold_dual_contouring->tree, manifold_dual_contouring->mesh->vertices, manifold_dual_contouring->vertice_pool);  // Finds normals and puts vertices in buffer
  manifold_octree_process_cell(manifold_dual_contouring->tree, manifold_dual_contouring->mesh->indices, threshold, manifold_dual_contouring->vertice_pool);  // Finds indices and puts indices in buffer

  if (vector_size(manifold_dual_contouring->mesh->vertices) > 0)
    manifold_dual_contouring_setup_buffers(manifold_dual_contouring, api_common);
}

internal inline b8 manifold_octree_construct_leaf(struct ManifoldOctreeNode* octree_node, struct Vector* vertice_pool, struct NoiseModule* planet_shape, int size, int scale, ivec3 origin);
internal inline void manifold_octree_process_face(struct ManifoldOctreeNode* nodes[2], int direction, struct Vector* indexes, r32 threshold, struct Vector* vertice_pool);
internal inline void manifold_octree_process_edge(struct ManifoldOctreeNode* nodes[4], int direction, struct Vector* indexes, r32 threshold, struct Vector* vertice_pool);
internal inline void manifold_octree_process_indexes(struct ManifoldOctreeNode* nodes[4], int direction, struct Vector* indexes, r32 threshold, struct Vector* vertice_pool);
internal inline void manifold_octree_cluster_cell(struct ManifoldOctreeNode* octree_node, r32 error, struct Vector* vertice_pool, struct NoiseModule* planet_shape);
internal inline void manifold_octree_gather_vertices(struct ManifoldOctreeNode* n, struct ArrayList* dest, int* surface_index);
internal inline void manifold_octree_cluster_face(struct ManifoldOctreeNode* nodes[2], int direction, int* surface_index, struct ArrayList* collected_vertices, struct Vector* vertice_pool);
internal inline void manifold_octree_cluster_edge(struct ManifoldOctreeNode* nodes[4], int direction, int* surface_index, struct ArrayList* collected_vertices, struct Vector* vertice_pool);
internal inline void manifold_octree_cluster_indexes(struct ManifoldOctreeNode* nodes[8], int direction, int* max_surface_index, struct ArrayList* collected_vertices, struct Vector* vertice_pool);

void manifold_octree_delete(struct ManifoldOctreeNode* octree_node) {
  if (!octree_node)
    return;

  for (int i = 0; i < 8; i++)
    manifold_octree_delete(octree_node->children[i]);
}

struct VertexFound {
  struct VertexManifoldDualContouring vertex_data;
  struct Vertex* vertex_handle;
};

internal inline void manifold_octree_generate_vertex_buffer_thread_split(struct ManifoldOctreeNode* octree_node, struct Vector* vertices, struct Vector* vertice_pool) {
  if (octree_node->type != MANIFOLD_NODE_LEAF) {
    for (int i = 0; i < 8; i++) {
      if (octree_node->children[i] != NULL)
        manifold_octree_generate_vertex_buffer_thread_split(octree_node->children[i], vertices, vertice_pool);
    }
  }

  if (vertices == NULL || octree_node->total_vertices == 0)
    return;

  for (int i = 0; i < octree_node->total_vertices; i++) {
    struct Vertex* ver = (struct Vertex*)vector_get(vertice_pool, octree_node->vertices[i]);
    if (ver == NULL)
      continue;
    vec3 nc = vec3_old_skool_normalise(vec3_add(vec3_scale(ver->normal, 0.5f), vec3_scale(VEC3_ONE, 0.5f)));
    vec3 solved_x = VEC3_ZERO;
    // TODO: Why is this being solved so many times? Just save results
    qef_solver_solve(&ver->qef, &solved_x, 1e-6f, 4, 1e-6f);
    vector_push_back(vertices, (struct VertexFound[]){(struct VertexFound){.vertex_data = (struct VertexManifoldDualContouring){.position.x = solved_x.x, .position.y = solved_x.y, .position.z = solved_x.z, .color.r = nc.r, .color.g = nc.g, .color.b = nc.b, .normal1.r = ver->normal.r, .normal1.g = ver->normal.g, .normal1.b = ver->normal.b, .normal2.r = ver->normal.r, .normal2.g = ver->normal.g, .normal2.b = ver->normal.b}, .vertex_handle = ver}});
  }
}

void manifold_octree_generate_vertex_buffer(struct ManifoldOctreeNode* octree_node, struct Vector* vertices, struct Vector* vertice_pool) {
  struct Vector vert_vectors[8] = {0};
  for (int vector_num = 0; vector_num < 8; vector_num++)
    vector_init(&vert_vectors[vector_num], sizeof(struct VertexFound));

  // Spawn thread for each octree child
  if (octree_node->type != MANIFOLD_NODE_LEAF) {
    for (int i = 0; i < 8; i++) {
      if (octree_node->children[i] != NULL)
        manifold_octree_generate_vertex_buffer_thread_split(octree_node->children[i], &vert_vectors[i], vertice_pool);
    }
  }

  // Combine child vectors
  for (int vector_num = 0; vector_num < 8; vector_num++) {
    for (int vert_num = 0; vert_num < vector_size(&vert_vectors[vector_num]); vert_num++) {
      struct VertexFound* new_vertex = vector_get(&vert_vectors[vector_num], vert_num);
      new_vertex->vertex_handle->index = vector_size(vertices);
      mesh_manifold_dual_contouring_assign_vertex_simple(vertices, new_vertex->vertex_data);
    }
  }

  for (int vector_num = 0; vector_num < 8; vector_num++)
    vector_delete(&vert_vectors[vector_num]);
}

void manifold_octree_init(struct ManifoldOctreeNode* tree, int resolution, int scale, struct ManifoldOctreeNode* node_cache, struct Vector* vertice_pool, struct Vector* node_pool, struct NoiseModule* planet_shape) {
  vector_push_back(node_pool, &(struct ManifoldOctreeNode){0});
  struct ManifoldOctreeNode* octree_node = vector_get(node_pool, node_pool->size);
  tree->children[0] = octree_node;

  octree_node->index = 0;
  octree_node->position = (ivec3){.x = TCornerDeltas[0].x * scale * MANIFOLD_RESOLUTION, .y = TCornerDeltas[0].y * scale * MANIFOLD_RESOLUTION, .z = TCornerDeltas[0].z * scale * MANIFOLD_RESOLUTION};
  octree_node->size = resolution;
  octree_node->type = MANIFOLD_NODE_INTERNAL;
  octree_node->child_index = 0;
  //
  int levels = 0;
  for (int current_level = octree_node->size; current_level > 0; current_level /= 2)
    levels++;

  if (levels <= 1)
    return;

  memset(octree_node->node_cache, 0, sizeof(struct ManifoldOctreeNode*) * levels);
  int total_nodes[MANIFOLD_MAX_OCTREE_LEVELS];
  memset(total_nodes, 0, sizeof(int) * levels);

  total_nodes[0] = 1;
  octree_node->node_cache[0] = octree_node;
  for (int series_num = 1; series_num < levels; series_num++) {
    total_nodes[series_num] = pow(8, series_num);
    octree_node->node_cache[series_num] = calloc(1, sizeof(struct ManifoldOctreeNode) * pow(8, series_num));
  }

  // Note: Needed to calculate position of lowest level
  for (int node_level = 1; node_level < levels - 1; node_level++) {
    const int child_size = octree_node->size / pow(2, node_level);
    for (int node_num = 0; node_num < total_nodes[node_level]; node_num++) {
      int index = node_num % 8;
      struct ManifoldOctreeNode* new_node = &octree_node->node_cache[node_level][node_num];
      octree_node_init(new_node, ivec3_add(octree_node->node_cache[node_level - 1][node_num / 8].position, ivec3_scale(TCornerDeltas[index], child_size)), child_size, scale, MANIFOLD_NODE_INTERNAL);
    }
  }

  for (int node_num = 0; node_num < total_nodes[levels - 1]; node_num++) {
    const unsigned int thread_id = omp_get_thread_num();
    int index = node_num % 8;
    struct ManifoldOctreeNode* new_node = &octree_node->node_cache[levels - 1][node_num];
    octree_node_init(new_node, ivec3_add(octree_node->node_cache[levels - 2][node_num / 8].position, TCornerDeltas[index]), 1, scale, MANIFOLD_NODE_INTERNAL);
    if (!manifold_octree_construct_leaf(new_node, &vertice_reduction_pools[thread_id], planet_shape, octree_node->size, scale, octree_node->position))
      new_node->type = MANIFOLD_NODE_NONE;
  }

  for (int node_level = levels - 2; node_level >= 0; node_level--) {
    for (int node_num = 0; node_num < total_nodes[node_level]; node_num++) {
      int index = node_num % 8;
      b8 has_children = FALSE;
      struct ManifoldOctreeNode* got_node = &octree_node->node_cache[node_level][node_num];

      for (int index = 0; index < 8; index++) {
        struct ManifoldOctreeNode* found_node = &octree_node->node_cache[node_level + 1][(node_num * 8) + index];
        if (found_node->type == MANIFOLD_NODE_NONE)
          continue;
        has_children |= TRUE;
        got_node->children[index] = found_node;
        got_node->children[index]->child_index = index;
      }

      got_node->type = (has_children) ? MANIFOLD_NODE_INTERNAL : MANIFOLD_NODE_NONE;
    }
  }
}

// TODO: Optimize below as much as possible
internal inline b8 manifold_octree_construct_leaf(struct ManifoldOctreeNode* octree_node, struct Vector* vertice_pool, struct NoiseModule* planet_shape, int size, int scale, ivec3 origin) {
  octree_node->type = MANIFOLD_NODE_LEAF;
  int corners = 0;
  r32 samples[8] = {0};

  // TODO: Majority of performance cost comes from eval and normals
  // Note: Once above is made faster move on unless needs to be even faster
  for (int sample_num = 0; sample_num < 8; sample_num++) {
    ivec3 sample_position = ivec3_add(origin, ivec3_scale(ivec3_sub(ivec3_add(octree_node->position, TCornerDeltas[sample_num]), origin), scale));
    samples[sample_num] = noise_module_eval(planet_shape, sample_position.x, sample_position.y, sample_position.z);
    if (samples[sample_num] < 0)
      corners |= 1 << sample_num;
  }

  // Loop through all corners if all FALSE then return
  octree_node->corners = (unsigned char)corners;
  if (corners == 0 || corners == 255)
    return FALSE;

  int total_edges = TransformedVerticesNumberTable[octree_node->corners];
  int v_edges[4][16] = {0};

  int v_index = 0;
  int e_index = 0;
  memset(v_edges[0], -1, sizeof(int) * 8);
  for (int e = 0; e < 16; e++) {
    int code = TransformedEdgesTable[corners][e];
    if (code == -2) {
      v_index++;
      break;
    }
    if (code == -1) {
      v_index++;
      e_index = 0;
      memset(v_edges[v_index], -1, sizeof(int) * 13);
      continue;
    }

    v_edges[v_index][e_index++] = code;
  }

  // memset(octree_node->vertices, 0, sizeof(octree_node->vertices));
  int swap = 0;
  // Note: Adds about 15% to performance cost could be worth making faster if a problem
  for (int i = 0; i < v_index; i++) {
    int k = 0;

    struct Vertex* get_vertice;
    vector_push_back(vertice_pool, &(struct Vertex){0});
    int index_num = vertice_pool->size - 1;
    get_vertice = (struct Vertex*)vector_get(vertice_pool, index_num);
    octree_node->vertices[octree_node->total_vertices++] = index_num;
    // get_vertice->vertice_reduction = &octree_node->vertices[octree_node->total_vertices - 1];
    get_vertice->vertice_reduction = octree_node->vertices + (octree_node->total_vertices - 1);

    vertex_init(get_vertice);
    qef_solver_init(&get_vertice->qef);
    vec3 normal = VEC3_ZERO;
    int ei[12] = {0};
    while (v_edges[i][k] != -1) {
      ei[v_edges[i][k]] = 1;

      ivec3 qef_pos = ivec3_add(origin, ivec3_scale(ivec3_sub(octree_node->position, origin), scale));
      ivec3 a = ivec3_add(qef_pos, ivec3_scale(TCornerDeltas[TEdgePairs[v_edges[i][k]][0]], octree_node->size * scale));
      ivec3 b = ivec3_add(qef_pos, ivec3_scale(TCornerDeltas[TEdgePairs[v_edges[i][k]][1]], octree_node->size * scale));
      vec3 intersection = vec3_add(ivec3_to_vec3(a), vec3_divs(vec3_scale(ivec3_to_vec3(ivec3_sub(b, a)), -samples[TEdgePairs[v_edges[i][k]][0]]), samples[TEdgePairs[v_edges[i][k]][1]] - samples[TEdgePairs[v_edges[i][k]][0]]));
      vec3 n = planet_normal(intersection, planet_shape, scale);
      normal = vec3_add(normal, n);
      qef_solver_add_simp(&get_vertice->qef, intersection, n);
      k++;
    }

    normal = vec3_old_skool_divs(normal, k);
    normal = vec3_old_skool_normalise(normal);
    get_vertice->index = octree_node->total_vertices;
    get_vertice->parent = NULL;
    get_vertice->collapsible = TRUE;
    get_vertice->normal = normal;
    get_vertice->euler = 1;
    memcpy(get_vertice->eis, ei, sizeof(int) * 12);
    get_vertice->in_cell = octree_node->child_index;
    get_vertice->face_prop2 = TRUE;
    vec3 emp_buffer = VEC3_ZERO;
    // Note: Barely adds like 10% time could be worth making faster later but not priority
    qef_solver_solve(&get_vertice->qef, &emp_buffer, 1e-6f, 4, 1e-6f);
    get_vertice->error = qef_solver_get_error_pos(&get_vertice->qef, get_vertice->qef.x);
  }

  return TRUE;
}
// TODO: Optimize above as much as possible

internal inline void manifold_octree_process_cell_split_threads(struct ManifoldOctreeNode* octree_node, struct Vector* indexes, r32 threshold, struct Vector* vertice_pool) {
  if (octree_node->type == MANIFOLD_NODE_INTERNAL) {
    for (int i = 0; i < 8; i++) {
      if (octree_node->children[i] != NULL)
        manifold_octree_process_cell_split_threads(octree_node->children[i], indexes, threshold, vertice_pool);
    }

    for (int i = 0; i < 12; i++) {
      struct ManifoldOctreeNode* face_nodes[2] = {NULL};

      int c1 = TEdgePairs[i][0];
      int c2 = TEdgePairs[i][1];

      face_nodes[0] = octree_node->children[c1];
      face_nodes[1] = octree_node->children[c2];

      manifold_octree_process_face(face_nodes, TEdgePairs[i][2], indexes, threshold, vertice_pool);
    }

    for (int i = 0; i < 6; i++) {
      struct ManifoldOctreeNode* edge_nodes[4] = {octree_node->children[TCellProcEdgeMask[i][0]], octree_node->children[TCellProcEdgeMask[i][1]], octree_node->children[TCellProcEdgeMask[i][2]], octree_node->children[TCellProcEdgeMask[i][3]]};

      manifold_octree_process_edge(edge_nodes, TCellProcEdgeMask[i][4], indexes, threshold, vertice_pool);
    }
  }
}

void manifold_octree_process_cell(struct ManifoldOctreeNode* octree_node, struct Vector* indexes, r32 threshold, struct Vector* vertice_pool) {
  struct Vector index_vectors[8] = {0};
  for (int index_num = 0; index_num < 8; index_num++)
    vector_init(&index_vectors[index_num], sizeof(u32));

  // Spawn thread for each octree child
  if (octree_node->type == MANIFOLD_NODE_INTERNAL) {
    for (int i = 0; i < 8; i++) {
      if (octree_node->children[i] != NULL)
        manifold_octree_process_cell_split_threads(octree_node->children[i], &index_vectors[i], threshold, vertice_pool);
    }

    // Combine child indices
    for (int vector_num = 0; vector_num < 8; vector_num++) {
      for (int vert_num = 0; vert_num < vector_size(&index_vectors[vector_num]); vert_num++)
        mesh_assign_indice(indexes, *(u32*)vector_get(&index_vectors[vector_num], vert_num));
    }

    for (int i = 0; i < 12; i++) {
      struct ManifoldOctreeNode* face_nodes[2] = {NULL};

      int c1 = TEdgePairs[i][0];
      int c2 = TEdgePairs[i][1];

      face_nodes[0] = octree_node->children[c1];
      face_nodes[1] = octree_node->children[c2];

      manifold_octree_process_face(face_nodes, TEdgePairs[i][2], indexes, threshold, vertice_pool);
    }

    for (int i = 0; i < 6; i++) {
      struct ManifoldOctreeNode* edge_nodes[4] = {octree_node->children[TCellProcEdgeMask[i][0]], octree_node->children[TCellProcEdgeMask[i][1]], octree_node->children[TCellProcEdgeMask[i][2]], octree_node->children[TCellProcEdgeMask[i][3]]};

      manifold_octree_process_edge(edge_nodes, TCellProcEdgeMask[i][4], indexes, threshold, vertice_pool);
    }
  }

  for (int index_num = 0; index_num < 8; index_num++)
    vector_delete(&index_vectors[index_num]);
}

internal inline void manifold_octree_process_face(struct ManifoldOctreeNode* nodes[2], int direction, struct Vector* indexes, r32 threshold, struct Vector* vertice_pool) {
  if (nodes[0] == NULL || nodes[1] == NULL)
    return;

  if (nodes[0]->type != MANIFOLD_NODE_LEAF || nodes[1]->type != MANIFOLD_NODE_LEAF) {
    for (int i = 0; i < 4; i++) {
      struct ManifoldOctreeNode* face_nodes[2] = {NULL};

      for (int j = 0; j < 2; j++) {
        if (nodes[j]->type == MANIFOLD_NODE_LEAF)
          face_nodes[j] = nodes[j];
        else
          face_nodes[j] = nodes[j]->children[TFaceProcFaceMask[direction][i][j]];
      }

      manifold_octree_process_face(face_nodes, TFaceProcFaceMask[direction][i][2], indexes, threshold, vertice_pool);
    }

    int orders[2][4] = {{0, 0, 1, 1}, {0, 1, 0, 1}};

    for (int i = 0; i < 4; i++) {
      struct ManifoldOctreeNode* edge_nodes[4] = {NULL};

      for (int j = 0; j < 4; j++) {
        if (nodes[orders[TFaceProcEdgeMask[direction][i][0]][j]]->type == MANIFOLD_NODE_LEAF)
          edge_nodes[j] = nodes[orders[TFaceProcEdgeMask[direction][i][0]][j]];
        else
          edge_nodes[j] = nodes[orders[TFaceProcEdgeMask[direction][i][0]][j]]->children[TFaceProcEdgeMask[direction][i][1 + j]];
      }

      manifold_octree_process_edge(edge_nodes, TFaceProcEdgeMask[direction][i][5], indexes, threshold, vertice_pool);
    }
  }
}

internal inline void manifold_octree_process_edge(struct ManifoldOctreeNode* nodes[4], int direction, struct Vector* indexes, r32 threshold, struct Vector* vertice_pool) {
  if (nodes[0] == NULL || nodes[1] == NULL || nodes[2] == NULL || nodes[3] == NULL)
    return;

  if (nodes[0]->type == MANIFOLD_NODE_LEAF && nodes[1]->type == MANIFOLD_NODE_LEAF && nodes[2]->type == MANIFOLD_NODE_LEAF && nodes[3]->type == MANIFOLD_NODE_LEAF) {
    manifold_octree_process_indexes(nodes, direction, indexes, threshold, vertice_pool);
  } else {
    for (int i = 0; i < 2; i++) {
      struct ManifoldOctreeNode* edge_nodes[4] = {NULL};

      for (int j = 0; j < 4; j++) {
        if (nodes[j]->type == MANIFOLD_NODE_LEAF)
          edge_nodes[j] = nodes[j];
        else
          edge_nodes[j] = nodes[j]->children[TEdgeProcEdgeMask[direction][i][j]];
      }

      manifold_octree_process_edge(edge_nodes, TEdgeProcEdgeMask[direction][i][4], indexes, threshold, vertice_pool);
    }
  }
}

internal inline void manifold_octree_process_indexes(struct ManifoldOctreeNode* nodes[4], int direction, struct Vector* indexes, r32 threshold, struct Vector* vertice_pool) {
  int min_size = 10000000;
  int indices[4] = {-1, -1, -1, -1};
  b8 flip = FALSE;
  b8 sign_changed = FALSE;

  for (int i = 0; i < 4; i++) {
    int edge = TProcessEdgeMask[direction][i];
    int c1 = TEdgePairs[edge][0];
    int c2 = TEdgePairs[edge][1];

    int m1 = (nodes[i]->corners >> c1) & 1;
    int m2 = (nodes[i]->corners >> c2) & 1;

    if (nodes[i]->size < min_size) {
      min_size = nodes[i]->size;
      flip = m1 == 1;
      sign_changed = ((m1 == 0 && m2 != 0) || (m1 != 0 && m2 == 0));
    }

    int index = 0;
    b8 skip = FALSE;
    for (int k = 0; k < 16; k++) {
      int e = TransformedEdgesTable[nodes[i]->corners][k];
      if (e == -1) {
        index++;
        continue;
      }
      if (e == -2) {
        // TODO: Figure out cases where different scaling adds more corners
        if (nodes[i]->scale == 1)
          skip = TRUE;
        break;
      }
      if (e == edge)
        break;
    }

    if (skip)
      continue;

    if (index >= nodes[i]->total_vertices)
      return;

    struct Vertex* v = (struct Vertex*)vector_get(vertice_pool, nodes[i]->vertices[index]);
    struct Vertex* highest = v;
    while (highest->parent != NULL) {
      if (highest->parent->error <= threshold && (highest->parent->euler == 1 && highest->parent->face_prop2))
        highest = v = highest->parent;
      else
        highest = highest->parent;
    }

    indices[i] = v->index;
  }

  if (sign_changed) {
    // Note: Problem is caused by wrong indice assignment
    // if (indices[3] == -1)
    //  return;
    if (!flip) {
      if (indices[0] != -1 && indices[1] != -1 && indices[2] != -1 && indices[0] != indices[1] && indices[1] != indices[3]) {
        mesh_assign_indice(indexes, indices[0]);
        mesh_assign_indice(indexes, indices[1]);
        mesh_assign_indice(indexes, indices[3]);
      }

      if (indices[0] != -1 && indices[2] != -1 && indices[3] != -1 && indices[0] != indices[2] && indices[2] != indices[3]) {
        mesh_assign_indice(indexes, indices[0]);
        mesh_assign_indice(indexes, indices[3]);
        mesh_assign_indice(indexes, indices[2]);
      }
    } else {
      if (indices[0] != -1 && indices[3] != -1 && indices[1] != -1 && indices[0] != indices[1] && indices[1] != indices[3]) {
        mesh_assign_indice(indexes, indices[0]);
        mesh_assign_indice(indexes, indices[3]);
        mesh_assign_indice(indexes, indices[1]);
      }

      if (indices[0] != -1 && indices[2] != -1 && indices[3] != -1 && indices[0] != indices[2] && indices[2] != indices[3]) {
        mesh_assign_indice(indexes, indices[0]);
        mesh_assign_indice(indexes, indices[2]);
        mesh_assign_indice(indexes, indices[3]);
      }
    }
  }
}

void manifold_octree_cluster_cell_base(struct ManifoldOctreeNode* octree_node, r32 error, struct Vector* vertice_pool, struct NoiseModule* planet_shape) {
  if (octree_node->type != MANIFOLD_NODE_INTERNAL)
    return;

  for (int i = 0; i < 8; i++) {
    if (octree_node->children[i] == NULL)
      continue;

    manifold_octree_cluster_cell(octree_node->children[i], error, vertice_pool, planet_shape);
  }
}

internal inline void manifold_octree_cluster_cell(struct ManifoldOctreeNode* octree_node, r32 error, struct Vector* vertice_pool, struct NoiseModule* planet_shape) {
  if (octree_node->type != MANIFOLD_NODE_INTERNAL)
    return;

  int signs[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
  int mid_sign = -1;

  for (int i = 0; i < 8; i++) {
    if (octree_node->children[i] == NULL)
      continue;

    manifold_octree_cluster_cell(octree_node->children[i], error, vertice_pool, planet_shape);
    if (octree_node->children[i]->type != MANIFOLD_NODE_INTERNAL) {
      mid_sign = (octree_node->children[i]->corners >> (7 - i)) & 1;
      signs[i] = (octree_node->children[i]->corners >> i) & 1;
    }
  }

  octree_node->corners = 0;
  for (int i = 0; i < 8; i++) {
    if (signs[i] == -1)
      octree_node->corners |= (unsigned char)(mid_sign << i);
    else
      octree_node->corners |= (unsigned char)(signs[i] << i);
  }

  int surface_index = 0;
  struct ArrayList collected_vertices = {0};
  array_list_init(&collected_vertices);
  unsigned char total_new_vertices = 0;
  unsigned int new_vertices[16] = {0};

  for (int i = 0; i < 12; i++) {
    struct ManifoldOctreeNode* face_nodes[2] = {NULL};

    int c1 = TEdgePairs[i][0];
    int c2 = TEdgePairs[i][1];

    face_nodes[0] = octree_node->children[c1];
    face_nodes[1] = octree_node->children[c2];

    manifold_octree_cluster_face(face_nodes, TEdgePairs[i][2], &surface_index, &collected_vertices, vertice_pool);
  }

  for (int i = 0; i < 6; i++) {
    struct ManifoldOctreeNode* edge_nodes[4] = {octree_node->children[TCellProcEdgeMask[i][0]], octree_node->children[TCellProcEdgeMask[i][1]], octree_node->children[TCellProcEdgeMask[i][2]], octree_node->children[TCellProcEdgeMask[i][3]]};
    manifold_octree_cluster_edge(edge_nodes, TCellProcEdgeMask[i][4], &surface_index, &collected_vertices, vertice_pool);
  }

  int highest_index = surface_index;

  if (highest_index == -1)
    highest_index = 0;

  for (int octree_num = 0; octree_num < 8; octree_num++) {
    struct ManifoldOctreeNode* n = octree_node->children[octree_num];
    if (n == NULL)
      continue;

    for (int vertice_num = 0; vertice_num < n->total_vertices; vertice_num++) {
      struct Vertex* v = (struct Vertex*)vector_get(vertice_pool, n->vertices[vertice_num]);
      if (v == NULL)
        continue;
      if (v->surface_index == -1) {
        v->surface_index = highest_index++;
        array_list_add(&collected_vertices, v);
      }
    }
  }

  if (array_list_size(&collected_vertices) > 0) {
    for (int i = 0; i <= highest_index; i++) {
      // printf("highest index %d\n", highest_index - 1);
      struct QefSolver qef = {0};
      qef_solver_init(&qef);
      vec3 normal = VEC3_ZERO;
      int count = 0;
      int edges[12] = {0};
      int euler = 0;
      int e = 0;

      for (int vertice_num = 0; vertice_num < array_list_size(&collected_vertices); vertice_num++) {
        struct Vertex* v = (struct Vertex*)array_list_get(&collected_vertices, vertice_num);
        if (v->surface_index == i) {
          for (int k = 0; k < 3; k++) {
            int edge = TExternalEdges[v->in_cell][k];
            edges[edge] += v->eis[edge];
          }
          for (int k = 0; k < 9; k++) {
            int edge = TInternalEdges[v->in_cell][k];
            e += v->eis[edge];
          }
          euler += v->euler;
          qef_solver_add_copy(&qef, &v->qef.data);
          normal = vec3_add(normal, v->normal);
          count++;
        }
      }

      if (count == 0)
        continue;

      b8 face_prop2 = TRUE;
      for (int f = 0; f < 6 && face_prop2; f++) {
        int intersections = 0;
        for (int ei = 0; ei < 4; ei++) {
          intersections += edges[TFaces[f][ei]];
        }
        if (!(intersections == 0 || intersections == 2))
          face_prop2 = FALSE;
      }

      vector_push_back(vertice_pool, &(struct Vertex){0});
      struct Vertex* new_vertex = (struct Vertex*)vector_get(vertice_pool, vertice_pool->size - 1);
      vertex_init(new_vertex);

      normal = vec3_old_skool_divs(normal, count);
      normal = vec3_old_skool_normalise(normal);
      new_vertex->normal = normal;
      new_vertex->qef = qef;
      memcpy(new_vertex->eis, edges, sizeof(int) * 12);
      new_vertex->euler = euler - e / 4;
      new_vertex->in_cell = octree_node->child_index;
      new_vertex->face_prop2 = face_prop2;
      int index_num = vertice_pool->size - 1;
      new_vertices[total_new_vertices++] = index_num;

      vec3 buf_extra = VEC3_ZERO;
      qef_solver_solve(&qef, &buf_extra, 1e-6f, 4, 1e-6f);
      r32 err = qef_solver_get_error(&qef);
      new_vertex->collapsible = err <= error;
      new_vertex->error = err;

      for (int vertice_num = 0; vertice_num < array_list_size(&collected_vertices); vertice_num++) {
        struct Vertex* v = (struct Vertex*)array_list_get(&collected_vertices, vertice_num);
        if (v->surface_index == i)
          v->parent = new_vertex;
      }
    }
  } else {
    array_list_delete(&collected_vertices);
    return;
  }

  for (int vertice_num = 0; vertice_num < array_list_size(&collected_vertices); vertice_num++) {
    struct Vertex* v2 = (struct Vertex*)array_list_get(&collected_vertices, vertice_num);
    v2->surface_index = -1;
  }

  octree_node->total_vertices = total_new_vertices;
  memcpy(octree_node->vertices, new_vertices, sizeof(unsigned int) * total_new_vertices);
  array_list_delete(&collected_vertices);
}

internal inline void manifold_octree_cluster_face(struct ManifoldOctreeNode* nodes[2], int direction, int* surface_index, struct ArrayList* collected_vertices, struct Vector* vertice_pool) {
  if (nodes[0] == NULL || nodes[1] == NULL)
    return;

  if (nodes[0]->type != MANIFOLD_NODE_LEAF || nodes[1]->type != MANIFOLD_NODE_LEAF) {
    for (int i = 0; i < 4; i++) {
      struct ManifoldOctreeNode* face_nodes[2] = {NULL};
      for (int j = 0; j < 2; j++) {
        if (nodes[j] == NULL)
          continue;
        if (nodes[j]->type != MANIFOLD_NODE_INTERNAL)
          face_nodes[j] = nodes[j];
        else
          face_nodes[j] = nodes[j]->children[TFaceProcFaceMask[direction][i][j]];
      }

      manifold_octree_cluster_face(face_nodes, TFaceProcFaceMask[direction][i][2], surface_index, collected_vertices, vertice_pool);
    }
  }

  int orders[2][4] = {{0, 0, 1, 1}, {0, 1, 0, 1}};

  for (int i = 0; i < 4; i++) {
    struct ManifoldOctreeNode* edge_nodes[4] = {NULL};
    for (int j = 0; j < 4; j++) {
      if (nodes[orders[TFaceProcEdgeMask[direction][i][0]][j]] == NULL)
        continue;
      if (nodes[orders[TFaceProcEdgeMask[direction][i][0]][j]]->type != MANIFOLD_NODE_INTERNAL)
        edge_nodes[j] = nodes[orders[TFaceProcEdgeMask[direction][i][0]][j]];
      else
        edge_nodes[j] = nodes[orders[TFaceProcEdgeMask[direction][i][0]][j]]->children[TFaceProcEdgeMask[direction][i][1 + j]];
    }

    manifold_octree_cluster_edge(edge_nodes, TFaceProcEdgeMask[direction][i][5], surface_index, collected_vertices, vertice_pool);
  }
}

internal inline void manifold_octree_cluster_edge(struct ManifoldOctreeNode* nodes[4], int direction, int* surface_index, struct ArrayList* collected_vertices, struct Vector* vertice_pool) {
  if ((nodes[0] == NULL || nodes[0]->type != MANIFOLD_NODE_INTERNAL) && (nodes[1] == NULL || nodes[1]->type != MANIFOLD_NODE_INTERNAL) && (nodes[2] == NULL || nodes[2]->type != MANIFOLD_NODE_INTERNAL) && (nodes[3] == NULL || nodes[3]->type != MANIFOLD_NODE_INTERNAL)) {
    manifold_octree_cluster_indexes(nodes, direction, surface_index, collected_vertices, vertice_pool);
  } else {
    for (int i = 0; i < 2; i++) {
      struct ManifoldOctreeNode* edge_nodes[4] = {NULL};
      for (int j = 0; j < 4; j++) {
        if (nodes[j] == NULL)
          continue;
        if (nodes[j]->type == MANIFOLD_NODE_LEAF)
          edge_nodes[j] = nodes[j];
        else
          edge_nodes[j] = nodes[j]->children[TEdgeProcEdgeMask[direction][i][j]];
      }

      manifold_octree_cluster_edge(edge_nodes, TEdgeProcEdgeMask[direction][i][4], surface_index, collected_vertices, vertice_pool);
    }
  }
}

internal inline void manifold_octree_cluster_indexes(struct ManifoldOctreeNode* nodes[8], int direction, int* max_surface_index, struct ArrayList* collected_vertices, struct Vector* vertice_pool) {
  if (nodes[0] == NULL && nodes[1] == NULL && nodes[2] == NULL && nodes[3] == NULL)
    return;

  struct Vertex* vertices[4] = {NULL};
  int v_count = 0;

  for (int i = 0; i < 4; i++) {
    if (nodes[i] == NULL)
      continue;

    int edge = TProcessEdgeMask[direction][i];
    int c1 = TEdgePairs[edge][0];
    int c2 = TEdgePairs[edge][1];

    int m1 = (nodes[i]->corners >> c1) & 1;
    int m2 = (nodes[i]->corners >> c2) & 1;

    int index = 0;
    b8 skip = FALSE;
    for (int k = 0; k < 16; k++) {
      int e = TransformedEdgesTable[nodes[i]->corners][k];
      if (e == -1) {
        index++;
        continue;
      }
      if (e == -2) {
        if (!((m1 == 0 && m2 != 0) || (m1 != 0 && m2 == 0)))
          skip = TRUE;
        break;
      }
      if (e == edge)
        break;
    }

    if (!skip && index < nodes[i]->total_vertices) {
      vertices[i] = (struct Vertex*)vector_get(vertice_pool, nodes[i]->vertices[index]);
      while (vertices[i]->parent != NULL)
        vertices[i] = vertices[i]->parent;
      v_count++;
    }
  }

  if (v_count == 0)
    return;

  int surface_index = -1;

  for (int i = 0; i < 4; i++) {
    struct Vertex* v = vertices[i];
    if (v == NULL)
      continue;

    if (v->surface_index != -1) {
      if (surface_index != -1 && surface_index != v->surface_index) {
        for (int vertex_num = 0; vertex_num < array_list_size(collected_vertices); vertex_num++) {
          struct Vertex* ver = (struct Vertex*)array_list_get(collected_vertices, vertex_num);
          if (ver != NULL && ver->surface_index == v->surface_index)
            ver->surface_index = surface_index;
        }
      } else if (surface_index == -1)
        surface_index = v->surface_index;
    }
  }

  if (surface_index == -1)
    surface_index = (*max_surface_index)++;

  for (int i = 0; i < 4; i++) {
    struct Vertex* v = vertices[i];
    if (v == NULL)
      continue;
    if (v->surface_index == -1)
      array_list_add(collected_vertices, v);
    v->surface_index = surface_index;
  }
}
*/
