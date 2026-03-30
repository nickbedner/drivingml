#include "mana/graphics/utilities/collada/modelgeometry.h"

struct Mesh* geometry_loader_extract_model_data(struct APICommon* api_common, struct XmlNode* geometry_node, struct Vector* vertex_weights, bool animated, bool inverted_y) {
  struct XmlNode* mesh_data = xml_node_get_child(xml_node_get_child(geometry_node, "geometry"), "mesh");
  struct ModelData* model_data = (struct ModelData*)malloc(sizeof(struct ModelData));
  model_data_init(model_data);

  geometry_loader_read_raw_data(model_data, mesh_data, vertex_weights, inverted_y);
  geometry_loader_assemble_vertices(model_data, mesh_data, inverted_y);
  geometry_loader_remove_unused_vertices(model_data);

  struct Mesh* model_mesh = (struct Mesh*)malloc(sizeof(struct Mesh));
  animated ? mesh_init(model_mesh, MESH_TYPE_MODEL, api_common) : mesh_init(model_mesh, MESH_TYPE_MODEL_STATIC, api_common);

  geometry_loader_convert_data_to_arrays(model_data, model_mesh, animated, inverted_y);

  // TODO: Create copy function and memiry size od items for vector
  // Todo: Also this code is whack
  free(model_mesh->mesh_common.indices->items);
  *(model_mesh->mesh_common.indices) = *(model_data->indices);
  model_mesh->mesh_common.indices->items = malloc(model_mesh->mesh_common.indices->memory_size * model_mesh->mesh_common.indices->capacity);
  memcpy(model_mesh->mesh_common.indices->items, model_data->indices->items, model_mesh->mesh_common.indices->memory_size * model_mesh->mesh_common.indices->capacity);
  // model_mesh->indices = model_data->indices;

  model_data_delete(model_data);
  free(model_data);

  return model_mesh;
}

void geometry_loader_read_raw_data(struct ModelData* model_data, struct XmlNode* mesh_data, struct Vector* vertex_weights, bool inverted_y) {
  geometry_loader_read_positions(model_data, mesh_data, vertex_weights);
  geometry_loader_read_normals(model_data, mesh_data);
  bool tex_coords_read = geometry_loader_read_texture_coordinates(model_data, mesh_data, inverted_y);
  geometry_loader_read_colors(model_data, mesh_data);
}

void geometry_loader_read_positions(struct ModelData* model_data, struct XmlNode* mesh_data, struct Vector* vertex_weights) {
  char* positions_id = xml_node_get_attribute(xml_node_get_child(xml_node_get_child(mesh_data, "vertices"), "input"), "source") + 1;
  struct XmlNode* positions_data = xml_node_get_child(xml_node_get_child_with_attribute(mesh_data, "source", "id", positions_id), "float_array");
  int32_t count = atoi(xml_node_get_attribute(positions_data, "count"));
  int32_t stride = atoi(xml_node_get_attribute(xml_node_get_child(xml_node_get_child(xml_node_get_child_with_attribute(mesh_data, "source", "id", positions_id), "technique_common"), "accessor"), "stride"));

  char* raw_data = _strdup(xml_node_get_data(positions_data));
  char* next_token = NULL;
  char* raw_part = strtok_s(raw_data, " ", &next_token);
  for (size_t position_num = 0; position_num < (size_t)count && raw_part != NULL; position_num++) {
    vec4 position = {0};
    for (size_t dim_num = 0; dim_num < (size_t)stride; dim_num++) {
      position.data[dim_num] = (float)atof(raw_part);
      raw_part = strtok_s(NULL, " ", &next_token);
    }

    mat4 correction = mat4_rotate(MAT4_IDENTITY, degree_to_radian(-90.0f), (vec3){.data[0] = 1.0f, .data[1] = 0.0f, .data[2] = 0.0f});
    position = mat4_mul_vec4(correction, position);
    vec3 position_corrected = (vec3){.data[0] = position.data[0], .data[1] = position.data[1], .data[2] = position.data[2]};
    struct RawVertexModel raw_vertex = {0};
    if (vertex_weights != NULL)
      raw_vertex_model_init(&raw_vertex, (uint32_t)vector_size(model_data->vertices), position_corrected, (struct VertexSkinData*)vector_get(vertex_weights, vector_size(model_data->vertices)));
    else
      raw_vertex_model_init(&raw_vertex, (uint32_t)vector_size(model_data->vertices), position_corrected, NULL);
    vector_push_back(model_data->vertices, &raw_vertex);
  }

  free(raw_data);
}

void geometry_loader_read_normals(struct ModelData* model_data, struct XmlNode* mesh_data) {
  struct XmlNode* material_node = xml_node_get_child(mesh_data, "polylist");
  if (material_node == NULL)
    material_node = xml_node_get_child(mesh_data, "triangles");
  // Note: Not sure why but Maya exports sphere mesh incorrectly
  // if (xml_node_get_child_with_attribute(material_node, "input", "semantic", "NORMAL") == NULL)
  //  material_node = xml_node_get_child(mesh_data, "vertices");

  char* normals_id = xml_node_get_attribute(xml_node_get_child_with_attribute(material_node, "input", "semantic", "NORMAL"), "source") + 1;
  struct XmlNode* normals_data = xml_node_get_child(xml_node_get_child_with_attribute(mesh_data, "source", "id", normals_id), "float_array");
  int32_t count = atoi(xml_node_get_attribute(normals_data, "count"));
  int32_t stride = atoi(xml_node_get_attribute(xml_node_get_child(xml_node_get_child(xml_node_get_child_with_attribute(mesh_data, "source", "id", normals_id), "technique_common"), "accessor"), "stride"));

  char* raw_data = _strdup(xml_node_get_data(normals_data));
  char* next_token = NULL;
  char* raw_part = strtok_s(raw_data, " ", &next_token);
  for (size_t normal_num = 0; normal_num < (size_t)count && raw_part != NULL; normal_num++) {
    vec4 normal = {0};
    for (size_t dim_num = 0; dim_num < (size_t)stride; dim_num++) {
      normal.data[dim_num] = (float)atof(raw_part);
      raw_part = strtok_s(NULL, " ", &next_token);
    }

    mat4 correction = mat4_rotate(MAT4_IDENTITY, degree_to_radian(-90.0f), (vec3){.data[0] = 1.0f, .data[1] = 0.0f, .data[2] = 0.0f});
    normal = mat4_mul_vec4(correction, normal);
    vec3 normal_corrected = (vec3){.data[0] = normal.data[0], .data[1] = normal.data[1], .data[2] = normal.data[2]};
    vector_push_back(model_data->normals, &normal_corrected);
  }

  free(raw_data);
}

bool geometry_loader_read_texture_coordinates(struct ModelData* model_data, struct XmlNode* mesh_data, bool inverted_y) {
  struct XmlNode* primitive = xml_node_get_child(mesh_data, "polylist");
  if (primitive == NULL)
    primitive = xml_node_get_child(mesh_data, "triangles");
  if (primitive == NULL)
    return false;

  struct XmlNode* tex_input = xml_node_get_child_with_attribute(primitive, "input", "semantic", "TEXCOORD");
  if (tex_input == NULL)
    return false;  // no UVs on this mesh

  const char* source_attr = xml_node_get_attribute(tex_input, "source");
  if (source_attr == NULL || source_attr[0] != '#')
    return false;

  const char* tex_coords_id = source_attr + 1;

  struct XmlNode* source_node = xml_node_get_child_with_attribute(mesh_data, "source", "id", tex_coords_id);
  if (source_node == NULL)
    return false;

  struct XmlNode* tex_coords_data = xml_node_get_child(source_node, "float_array");
  if (tex_coords_data == NULL)
    return false;

  struct XmlNode* technique_common = xml_node_get_child(source_node, "technique_common");
  if (technique_common == NULL)
    return false;

  struct XmlNode* accessor = xml_node_get_child(technique_common, "accessor");
  if (accessor == NULL)
    return false;

  int32_t count = atoi(xml_node_get_attribute(accessor, "count"));
  int32_t stride = atoi(xml_node_get_attribute(accessor, "stride"));
  if (stride <= 0 || stride > 4)
    return false;

  char* raw_data = _strdup(xml_node_get_data(tex_coords_data));
  if (raw_data == NULL)
    return false;

  char* next_token = NULL;
  char* raw_part = strtok_s(raw_data, " \t\r\n", &next_token);

  for (int32_t tex_coord_num = 0; tex_coord_num < count; tex_coord_num++) {
    vec4 tex_coord = {0};

    for (int32_t dim_num = 0; dim_num < stride; dim_num++) {
      if (raw_part == NULL) {
        free(raw_data);
        return false;
      }

      float v = (float)atof(raw_part);
      tex_coord.data[dim_num] = (inverted_y && dim_num == 1) ? (1.0f - v) : v;
      raw_part = strtok_s(NULL, " \t\r\n", &next_token);
    }

    vector_push_back(model_data->tex_coords, &tex_coord);
  }

  free(raw_data);
  return true;
}

void geometry_loader_read_colors(struct ModelData* model_data, struct XmlNode* mesh_data) {
  struct XmlNode* material_node = xml_node_get_child(mesh_data, "polylist");
  if (material_node == NULL)
    material_node = xml_node_get_child(mesh_data, "triangles");
  // if (xml_node_get_child_with_attribute(material_node, "input", "semantic", "COLOR") == NULL)
  //   material_node = xml_node_get_child(mesh_data, "vertices");

  struct XmlNode* colors_location = xml_node_get_child_with_attribute(material_node, "input", "semantic", "COLOR");
  if (colors_location == NULL) {
    vec3 color = (vec3){.r = 0.0, .g = 0.0, .b = 0.0};
    vector_push_back(model_data->colors, &color);
    return;
  }
  char* colors_id = xml_node_get_attribute(colors_location, "source") + 1;
  struct XmlNode* colors_data = xml_node_get_child(xml_node_get_child_with_attribute(mesh_data, "source", "id", colors_id), "float_array");
  // int32_t count = atoi(xml_node_get_attribute(colors_data, "count"));
  int32_t stride = atoi(xml_node_get_attribute(xml_node_get_child(xml_node_get_child(xml_node_get_child_with_attribute(mesh_data, "source", "id", colors_id), "technique_common"), "accessor"), "stride"));

  char* raw_data = _strdup(xml_node_get_data(colors_data));
  char* next_token = NULL;
  char* raw_part = strtok_s(raw_data, " ", &next_token);
  while (raw_part != NULL) {
    vec4 color = {0};
    for (size_t type_num = 0; type_num < (size_t)stride; type_num++) {
      switch (type_num) {
        case 0: {
          color.r = (float)atof(raw_part);
          break;
        }
        case 1: {
          color.g = (float)atof(raw_part);
          break;
        }
        case 2: {
          color.b = (float)atof(raw_part);
          break;
        }
        case 3: {
          color.a = (float)atof(raw_part);
          break;
        }
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcovered-switch-default"
        default: {
          __builtin_unreachable();
        }
#pragma clang diagnostic pop
      }
      raw_part = strtok_s(NULL, " ", &next_token);
    }

    vector_push_back(model_data->colors, &color);
  }
  free(raw_data);
}

// Note: All collada models must follow this format and can only be 1 object
// Maybe add error checking to ignore any extras
void geometry_loader_assemble_vertices(struct ModelData* model_data, struct XmlNode* mesh_data, bool inverted_y) {
  struct XmlNode* poly = xml_node_get_child(mesh_data, "polylist");
  if (poly == NULL)
    poly = xml_node_get_child(mesh_data, "triangles");

  struct XmlNode* index_data = xml_node_get_child(poly, "p");
  size_t type_count = array_list_size(xml_node_get_children(poly, "input"));

  char* raw_data = _strdup(xml_node_get_data(index_data));
  char* next_token = NULL;
  char* raw_part = strtok_s(raw_data, " ", &next_token);
  while (raw_part != NULL) {
    int32_t position_index = 0, normal_index = 0, tex_coord_index = 0, color_index = 0;

    for (size_t type_num = 0; type_num < type_count; type_num++) {
      switch (type_num) {
        case 0: {
          position_index = atoi(raw_part);
          break;
        }
        case 1: {
          normal_index = atoi(raw_part);
          break;
        }
        case 2: {
          tex_coord_index = atoi(raw_part);
          break;
        }
        case 3: {
          color_index = atoi(raw_part);
          break;
        }
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcovered-switch-default"
        default: {
          __builtin_unreachable();
        }
#pragma clang diagnostic pop
      }
      raw_part = strtok_s(NULL, " ", &next_token);
    }
    geometry_loader_process_vertex(model_data, position_index, normal_index, tex_coord_index, color_index);
  }
  free(raw_data);

  if (inverted_y == false) {
    size_t indices_count = vector_size(model_data->indices);
    for (size_t i = 0; i < indices_count / 2; i++) {
      int32_t* start_index = (int32_t*)vector_get(model_data->indices, i);
      int32_t* end_index = (int32_t*)vector_get(model_data->indices, indices_count - 1 - i);

      // Swap indices
      int32_t temp = *start_index;
      *start_index = *end_index;
      *end_index = temp;
    }
  }
}

void geometry_loader_process_vertex(struct ModelData* model_data, int32_t position_index, int32_t normal_index, int32_t tex_coord_index, int32_t color_index) {
  struct RawVertexModel* current_vertex = (struct RawVertexModel*)vector_get(model_data->vertices, (size_t)position_index);
  if (raw_vertex_model_is_set(current_vertex) == false) {
    current_vertex->texture_index = tex_coord_index;
    current_vertex->normal_index = normal_index;
    current_vertex->color_index = color_index;
    vector_push_back(model_data->indices, &position_index);
  } else
    geometry_loader_deal_with_already_processed_vertex(model_data, current_vertex, tex_coord_index, normal_index, color_index);
}

float geometry_loader_convert_data_to_arrays(struct ModelData* model_data, struct Mesh* model_mesh, bool animated, bool inverted_y) {
  float furthest_point = 0.0f;

  for (size_t vertex_num = 0; vertex_num < vector_size(model_data->vertices); vertex_num++) {
    struct RawVertexModel* current_vertex = (struct RawVertexModel*)vector_get(model_data->vertices, vertex_num);

    if (current_vertex == NULL)
      continue;

    if (current_vertex->length > furthest_point)
      furthest_point = current_vertex->length;

    vec3 model_position = current_vertex->position;

    /* Safe defaults for missing attributes */
    vec3 model_normal = (vec3){.x = 0.0f, .y = 0.0f, .z = 1.0f};
    vec3 model_color = (vec3){.x = 1.0f, .y = 1.0f, .z = 1.0f};
    vec2 model_tex_coord = (vec2){.x = 0.0f, .y = 0.0f};

    /* Normal */
    if (current_vertex->normal_index >= 0 && (size_t)current_vertex->normal_index < vector_size(model_data->normals)) {
      vec3* normal_ptr = (vec3*)vector_get(model_data->normals, (size_t)current_vertex->normal_index);
      if (normal_ptr != NULL)
        model_normal = *normal_ptr;
    }

    /* Color */
    if (current_vertex->color_index >= 0 && (size_t)current_vertex->color_index < vector_size(model_data->colors)) {
      vec3* color_ptr = (vec3*)vector_get(model_data->colors, (size_t)current_vertex->color_index);
      if (color_ptr != NULL)
        model_color = *color_ptr;
    }

    /* Texture coordinates */
    if (current_vertex->texture_index >= 0 && (size_t)current_vertex->texture_index < vector_size(model_data->tex_coords)) {
      vec2* tex_ptr = (vec2*)vector_get(model_data->tex_coords, (size_t)current_vertex->texture_index);
      if (tex_ptr != NULL)
        model_tex_coord = *tex_ptr;
    }

    if (inverted_y)
      model_tex_coord.v = 1.0f - model_tex_coord.v;

    if (animated) {
      ivec3 joint_ids = (ivec3){.x = 0, .y = 0, .z = 0};
      vec3 joint_weights = (vec3){.x = 0.0f, .y = 0.0f, .z = 0.0f};

      if (current_vertex->weights_data != NULL) {
        struct VertexSkinData* model_weights = current_vertex->weights_data;

        if (model_weights->joint_ids != NULL && model_weights->joint_ids->items != NULL && vector_size(model_weights->joint_ids) >= 3)
          joint_ids = *(ivec3*)model_weights->joint_ids->items;

        if (model_weights->weights != NULL && model_weights->weights->items != NULL && vector_size(model_weights->weights) >= 3)
          joint_weights = *(vec3*)model_weights->weights->items;
      }

      struct VertexModel new_vertice = (struct VertexModel){.position = model_position, .normal = model_normal, .tex_coord = model_tex_coord, .color = model_color, .joints_ids = joint_ids, .weights = joint_weights};
      mesh_assign_vertex(model_mesh, &new_vertice);
    } else {
      struct VertexModelStatic new_vertice = (struct VertexModelStatic){.position = model_position, .normal = model_normal, .tex_coord = model_tex_coord, .color = model_color};
      mesh_assign_vertex(model_mesh, &new_vertice);
    }
  }

  return furthest_point;
}

void geometry_loader_deal_with_already_processed_vertex(struct ModelData* model_data, struct RawVertexModel* previous_vertex, int32_t new_texture_index, int32_t new_normal_index, int32_t new_color_index) {
  if (raw_vertex_model_has_same_texture_and_normal(previous_vertex, new_texture_index, new_normal_index)) {
    vector_push_back(model_data->indices, &previous_vertex->index);
  } else {
    struct RawVertexModel* another_vertex = previous_vertex->duplicate_vertex;
    if (another_vertex != NULL) {
      geometry_loader_deal_with_already_processed_vertex(model_data, another_vertex, new_texture_index, new_normal_index, new_color_index);
    } else {
      struct RawVertexModel duplicate_vertex = {0};
      raw_vertex_model_init(&duplicate_vertex, (uint32_t)vector_size(model_data->vertices), previous_vertex->position, previous_vertex->weights_data);
      duplicate_vertex.texture_index = new_texture_index;
      duplicate_vertex.normal_index = new_normal_index;
      duplicate_vertex.color_index = new_color_index;
      vector_push_back(model_data->vertices, &duplicate_vertex);
      vector_push_back(model_data->indices, &duplicate_vertex.index);
      // TODO: Watch could cause problems
      previous_vertex->duplicate_vertex = (struct RawVertexModel*)vector_get(model_data->vertices, vector_size(model_data->vertices));
    }
  }
}

void geometry_loader_remove_unused_vertices(struct ModelData* model_data) {
  for (size_t vertex_num = 0; vertex_num < vector_size(model_data->vertices); vertex_num++) {
    struct RawVertexModel* vertex = (struct RawVertexModel*)vector_get(model_data->vertices, vertex_num);
    if (raw_vertex_model_is_set(vertex) == false) {
      vertex->texture_index = 0;
      vertex->normal_index = 0;
    }
  }
}
