#pragma once

#include <mana/core/graphics/entities/model/modelcommon.h>
#include <mana/core/graphics/utilities/mesh/mesh.h>
#include <mana/core/math/advmath.h>
#include <mana/core/storage/storage.h>
#include <mana/core/utilities/xmlparser.h>

#define NO_INDEX -1

internal inline void raw_vertex_model_init(struct RawVertexModel* raw_vertex_model, u32 index, vec3 position, struct VertexSkinData* weights_data) {
  raw_vertex_model->texture_index = NO_INDEX;
  raw_vertex_model->normal_index = NO_INDEX;
  raw_vertex_model->color_index = NO_INDEX;
  raw_vertex_model->index = index;
  raw_vertex_model->weights_data = weights_data;
  raw_vertex_model->position = position;
  raw_vertex_model->length = vec3_magnitude(position);
  raw_vertex_model->duplicate_vertex = NULL;
}

internal inline b8 raw_vertex_model_is_set(struct RawVertexModel* raw_vertex_model) {
  return raw_vertex_model->texture_index != NO_INDEX && raw_vertex_model->normal_index != NO_INDEX;
}

internal inline b8 raw_vertex_model_has_same_texture_and_normal(struct RawVertexModel* raw_vertex_model, i32 texture_index_other, i32 normal_index_other) {
  return texture_index_other == raw_vertex_model->texture_index && normal_index_other == raw_vertex_model->normal_index;
}

internal inline void model_data_init(struct ModelData* model_data) {
  model_data->vertices = (struct Vector*)malloc(sizeof(struct Vector));
  vector_init(model_data->vertices, sizeof(struct RawVertexModel));

  model_data->tex_coords = (struct Vector*)malloc(sizeof(struct Vector));
  vector_init(model_data->tex_coords, sizeof(vec2));

  model_data->normals = (struct Vector*)malloc(sizeof(struct Vector));
  vector_init(model_data->normals, sizeof(vec3));

  model_data->colors = (struct Vector*)malloc(sizeof(struct Vector));
  vector_init(model_data->colors, sizeof(vec3));

  model_data->indices = (struct Vector*)malloc(sizeof(struct Vector));
  vector_init(model_data->indices, sizeof(u32));

  model_data->joint_ids = (struct Vector*)malloc(sizeof(struct Vector));
  vector_init(model_data->joint_ids, sizeof(ivec3));

  model_data->vertex_weights = (struct Vector*)malloc(sizeof(struct Vector));
  vector_init(model_data->vertex_weights, sizeof(vec3));
}

internal inline void model_data_delete(struct ModelData* model_data) {
  vector_delete(model_data->vertex_weights);
  free(model_data->vertex_weights);

  vector_delete(model_data->joint_ids);
  free(model_data->joint_ids);

  vector_delete(model_data->indices);
  free(model_data->indices);

  vector_delete(model_data->colors);
  free(model_data->colors);

  vector_delete(model_data->normals);
  free(model_data->normals);

  vector_delete(model_data->tex_coords);
  free(model_data->tex_coords);

  vector_delete(model_data->vertices);
  free(model_data->vertices);
}

struct Mesh* geometry_loader_extract_model_data(struct APICommon* api_common, struct XmlNode* geometry_node, struct Vector* vertex_weights, b8 animated, b8 inverted_y);
void geometry_loader_read_raw_data(struct ModelData* model_data, struct XmlNode* mesh_data, struct Vector* vertex_weights, b8 inverted_y);
void geometry_loader_read_positions(struct ModelData* model_data, struct XmlNode* mesh_data, struct Vector* vertex_weights);
void geometry_loader_read_normals(struct ModelData* model_data, struct XmlNode* mesh_data);
void geometry_loader_read_colors(struct ModelData* model_data, struct XmlNode* mesh_data);
b8 geometry_loader_read_texture_coordinates(struct ModelData* model_data, struct XmlNode* mesh_data, b8 inverted_y);
void geometry_loader_assemble_vertices(struct ModelData* model_data, struct XmlNode* mesh_data, b8 inverted_y);
void geometry_loader_process_vertex(struct ModelData* model_data, int position_index, int normal_index, int tex_coord_index, int color_index);
r32 geometry_loader_convert_data_to_arrays(struct ModelData* model_data, struct Mesh* model_mesh, b8 animated, b8 inverted_y);
void geometry_loader_deal_with_already_processed_vertex(struct ModelData* model_data, struct RawVertexModel* previous_vertex, int new_texture_index, int new_normal_index, int new_color_index);
void geometry_loader_remove_unused_vertices(struct ModelData* model_data);
