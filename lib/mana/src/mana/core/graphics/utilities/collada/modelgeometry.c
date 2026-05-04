#include "mana/core/graphics/utilities/collada/modelgeometry.h"

struct GeometryPrimitiveInputs {
  i32 vertex_offset;
  i32 normal_offset;
  i32 tex_coord_offset;
  i32 color_offset;
  i32 stride;
};

static i32 geometry_loader_surface_type_from_primitive(struct XmlNode* primitive) {
  char* material = xml_node_get_attribute(primitive, "material");

  if (material == NULL)
    return TRACK_SURFACE_UNKNOWN;

  if (strstr(material, "grass") != NULL)
    return TRACK_SURFACE_GRASS;

  if (strstr(material, "road") != NULL)
    return TRACK_SURFACE_ROAD;

  if (strstr(material, "sand_drive") != NULL)
    return TRACK_SURFACE_SAND_DRIVE;

  if (strstr(material, "sand") != NULL)
    return TRACK_SURFACE_SAND;

  if (strstr(material, "wall") != NULL)
    return TRACK_SURFACE_WALL;

  if (strstr(material, "oob") != NULL || strstr(material, "out") != NULL)
    return TRACK_SURFACE_OOB;

  return TRACK_SURFACE_UNKNOWN;
}

static i32 geometry_loader_get_input_offset(struct XmlNode* primitive, const char* semantic) {
  struct XmlNode* input = xml_node_get_child_with_attribute(primitive, "input", "semantic", semantic);
  if (input == NULL)
    return -1;

  char* offset_str = xml_node_get_attribute(input, "offset");
  if (offset_str == NULL)
    return -1;

  return atoi(offset_str);
}

static struct GeometryPrimitiveInputs geometry_loader_get_primitive_inputs(struct XmlNode* primitive) {
  struct GeometryPrimitiveInputs inputs = {
      .vertex_offset = -1,
      .normal_offset = -1,
      .tex_coord_offset = -1,
      .color_offset = -1,
      .stride = 0};

  inputs.vertex_offset = geometry_loader_get_input_offset(primitive, "VERTEX");
  inputs.normal_offset = geometry_loader_get_input_offset(primitive, "NORMAL");
  inputs.tex_coord_offset = geometry_loader_get_input_offset(primitive, "TEXCOORD");
  inputs.color_offset = geometry_loader_get_input_offset(primitive, "COLOR");

  i32 max_offset = inputs.vertex_offset;
  if (inputs.normal_offset > max_offset)
    max_offset = inputs.normal_offset;
  if (inputs.tex_coord_offset > max_offset)
    max_offset = inputs.tex_coord_offset;
  if (inputs.color_offset > max_offset)
    max_offset = inputs.color_offset;

  inputs.stride = max_offset + 1;
  return inputs;
}

static struct Vector* geometry_loader_parse_i32_list(const char* text) {
  struct Vector* values = (struct Vector*)malloc(sizeof(struct Vector));
  vector_init(values, sizeof(i32));

  if (text == NULL)
    return values;

  char* raw_data = _strdup(text);
  char* next_token = NULL;
  char* raw_part = strtok_s(raw_data, " \t\r\n", &next_token);

  while (raw_part != NULL) {
    i32 parsed_value = atoi(raw_part);
    vector_push_back(values, &parsed_value);
    raw_part = strtok_s(NULL, " \t\r\n", &next_token);
  }

  free(raw_data);
  return values;
}

static i32 geometry_loader_get_corner_index(struct Vector* index_stream, size_t corner_index, i32 offset, i32 stride) {
  if (offset < 0 || stride <= 0)
    return -1;

  size_t flat_index = corner_index * (size_t)stride + (size_t)offset;
  if (flat_index >= vector_size(index_stream))
    return -1;

  return *(i32*)vector_get(index_stream, flat_index);
}

static void geometry_loader_emit_corner(struct ModelData* model_data, struct Vector* index_stream, size_t corner_index, struct GeometryPrimitiveInputs inputs) {
  i32 position_index = geometry_loader_get_corner_index(index_stream, corner_index, inputs.vertex_offset, inputs.stride);
  i32 normal_index = geometry_loader_get_corner_index(index_stream, corner_index, inputs.normal_offset, inputs.stride);
  i32 tex_coord_index = geometry_loader_get_corner_index(index_stream, corner_index, inputs.tex_coord_offset, inputs.stride);
  i32 color_index = geometry_loader_get_corner_index(index_stream, corner_index, inputs.color_offset, inputs.stride);

  if (position_index < 0)
    return;

  geometry_loader_process_vertex(model_data, position_index, normal_index, tex_coord_index, color_index);
}

static b8 geometry_loader_vertex_matches(const struct RawVertexModel* vertex, i32 texture_index, i32 normal_index, i32 color_index) {
  return vertex->texture_index == texture_index && vertex->normal_index == normal_index && vertex->color_index == color_index;
}

static void geometry_loader_copy_vector(struct Vector* dst, struct Vector* src) {
  if (dst == NULL || src == NULL)
    return;

  if (dst->items != NULL)
    free(dst->items);

  *dst = *src;

  size_t byte_count = src->memory_size * src->capacity;

  if (byte_count > 0 && src->items != NULL) {
    dst->items = malloc(byte_count);
    memcpy(dst->items, src->items, byte_count);
  } else {
    dst->items = NULL;
  }
}

struct Mesh* geometry_loader_extract_model_data(struct APICommon* api_common, struct XmlNode* geometry_node, struct Vector* vertex_weights, b8 animated, b8 inverted_y) {
  struct XmlNode* geometry = xml_node_get_child(geometry_node, "geometry");
  if (geometry == NULL)
    return NULL;

  struct XmlNode* mesh_data = xml_node_get_child(geometry, "mesh");
  if (mesh_data == NULL)
    return NULL;

  struct ModelData* model_data = (struct ModelData*)malloc(sizeof(struct ModelData));
  model_data_init(model_data);

  geometry_loader_read_raw_data(model_data, mesh_data, vertex_weights, inverted_y);
  geometry_loader_assemble_vertices(model_data, mesh_data, inverted_y);
  geometry_loader_remove_unused_vertices(model_data);

  printf("MODEL_DATA: triangles=%zu surface_types=%zu\n", vector_size(model_data->indices) / 3, vector_size(model_data->surface_types));

  struct Mesh* model_mesh = (struct Mesh*)malloc(sizeof(struct Mesh));

  animated ? mesh_init(model_mesh, MESH_TYPE_MODEL, api_common) : mesh_init(model_mesh, MESH_TYPE_MODEL_STATIC, api_common);

  if (model_mesh->mesh_common.triangle_surface_types == NULL) {
    model_mesh->mesh_common.triangle_surface_types = (struct Vector*)malloc(sizeof(struct Vector));
    vector_init(model_mesh->mesh_common.triangle_surface_types, sizeof(i32));
  }

  geometry_loader_convert_data_to_arrays(model_data, model_mesh, animated, inverted_y);

  geometry_loader_copy_vector(model_mesh->mesh_common.indices, model_data->indices);

  geometry_loader_copy_vector(model_mesh->mesh_common.triangle_surface_types, model_data->surface_types);

  printf("MODEL_MESH: triangles=%zu surface_types=%zu\n", vector_size(model_mesh->mesh_common.indices) / 3, vector_size(model_mesh->mesh_common.triangle_surface_types));

  model_data_delete(model_data);
  free(model_data);

  return model_mesh;
}

void geometry_loader_read_raw_data(struct ModelData* model_data, struct XmlNode* mesh_data, struct Vector* vertex_weights, b8 inverted_y) {
  geometry_loader_read_positions(model_data, mesh_data, vertex_weights);
  geometry_loader_read_normals(model_data, mesh_data);
  geometry_loader_read_texture_coordinates(model_data, mesh_data, inverted_y);
  geometry_loader_read_colors(model_data, mesh_data);
}

void geometry_loader_read_positions(struct ModelData* model_data, struct XmlNode* mesh_data, struct Vector* vertex_weights) {
  char* positions_id = xml_node_get_attribute(xml_node_get_child(xml_node_get_child(mesh_data, "vertices"), "input"), "source") + 1;

  struct XmlNode* source_node = xml_node_get_child_with_attribute(mesh_data, "source", "id", positions_id);
  struct XmlNode* positions_data = xml_node_get_child(source_node, "float_array");
  struct XmlNode* accessor = xml_node_get_child(xml_node_get_child(source_node, "technique_common"), "accessor");

  i32 count = atoi(xml_node_get_attribute(accessor, "count"));
  i32 stride = atoi(xml_node_get_attribute(accessor, "stride"));

  char* raw_data = _strdup(xml_node_get_data(positions_data));
  char* next_token = NULL;
  char* raw_part = strtok_s(raw_data, " \t\r\n", &next_token);

  for (i32 position_num = 0; position_num < count && raw_part != NULL; position_num++) {
    vec4 position = {0};

    for (i32 dim_num = 0; dim_num < stride; dim_num++) {
      position.data[dim_num] = (r32)atof(raw_part);
      raw_part = strtok_s(NULL, " \t\r\n", &next_token);
    }

    mat4 correction = mat4_rotate(MAT4_IDENTITY, degree_to_radian(-90.0f), (vec3){.data[0] = 1.0f, .data[1] = 0.0f, .data[2] = 0.0f});
    position = mat4_mul_vec4(correction, position);

    vec3 position_corrected = {
        .data[0] = position.data[0],
        .data[1] = position.data[1],
        .data[2] = position.data[2]};

    struct RawVertexModel raw_vertex = {0};
    if (vertex_weights != NULL)
      raw_vertex_model_init(&raw_vertex, (u32)vector_size(model_data->vertices), position_corrected, (struct VertexSkinData*)vector_get(vertex_weights, vector_size(model_data->vertices)));
    else
      raw_vertex_model_init(&raw_vertex, (u32)vector_size(model_data->vertices), position_corrected, NULL);

    vector_push_back(model_data->vertices, &raw_vertex);
  }

  free(raw_data);
}

void geometry_loader_read_normals(struct ModelData* model_data, struct XmlNode* mesh_data) {
  struct XmlNode* material_node = xml_node_get_child(mesh_data, "polylist");
  if (material_node == NULL)
    material_node = xml_node_get_child(mesh_data, "triangles");
  if (material_node == NULL)
    return;

  struct XmlNode* normal_input = xml_node_get_child_with_attribute(material_node, "input", "semantic", "NORMAL");
  if (normal_input == NULL)
    return;

  char* source_attr = xml_node_get_attribute(normal_input, "source");
  if (source_attr == NULL || source_attr[0] != '#')
    return;

  char* normals_id = source_attr + 1;

  struct XmlNode* source_node = xml_node_get_child_with_attribute(mesh_data, "source", "id", normals_id);
  if (source_node == NULL)
    return;

  struct XmlNode* normals_data = xml_node_get_child(source_node, "float_array");
  struct XmlNode* technique_common = xml_node_get_child(source_node, "technique_common");
  struct XmlNode* accessor = xml_node_get_child(technique_common, "accessor");
  if (normals_data == NULL || accessor == NULL)
    return;

  i32 count = atoi(xml_node_get_attribute(accessor, "count"));
  i32 stride = atoi(xml_node_get_attribute(accessor, "stride"));

  char* raw_data = _strdup(xml_node_get_data(normals_data));
  if (raw_data == NULL)
    return;

  char* next_token = NULL;
  char* raw_part = strtok_s(raw_data, " \t\r\n", &next_token);

  for (i32 normal_num = 0; normal_num < count && raw_part != NULL; normal_num++) {
    vec4 normal = {0};

    for (i32 dim_num = 0; dim_num < stride; dim_num++) {
      normal.data[dim_num] = (r32)atof(raw_part);
      raw_part = strtok_s(NULL, " \t\r\n", &next_token);
    }

    mat4 correction = mat4_rotate(MAT4_IDENTITY, degree_to_radian(-90.0f), (vec3){.data[0] = 1.0f, .data[1] = 0.0f, .data[2] = 0.0f});
    normal = mat4_mul_vec4(correction, normal);

    vec3 normal_corrected = {
        .data[0] = normal.data[0],
        .data[1] = normal.data[1],
        .data[2] = normal.data[2]};

    vector_push_back(model_data->normals, &normal_corrected);
  }

  free(raw_data);
}

b8 geometry_loader_read_texture_coordinates(struct ModelData* model_data, struct XmlNode* mesh_data, b8 inverted_y) {
  struct XmlNode* primitive = xml_node_get_child(mesh_data, "polylist");
  if (primitive == NULL)
    primitive = xml_node_get_child(mesh_data, "triangles");
  if (primitive == NULL)
    return FALSE;

  struct XmlNode* tex_input = xml_node_get_child_with_attribute(primitive, "input", "semantic", "TEXCOORD");
  if (tex_input == NULL)
    return FALSE;  // no UVs on this mesh

  const char* source_attr = xml_node_get_attribute(tex_input, "source");
  if (source_attr == NULL || source_attr[0] != '#')
    return FALSE;

  const char* tex_coords_id = source_attr + 1;

  struct XmlNode* source_node = xml_node_get_child_with_attribute(mesh_data, "source", "id", tex_coords_id);
  if (source_node == NULL)
    return FALSE;

  struct XmlNode* tex_coords_data = xml_node_get_child(source_node, "float_array");
  if (tex_coords_data == NULL)
    return FALSE;

  struct XmlNode* technique_common = xml_node_get_child(source_node, "technique_common");
  if (technique_common == NULL)
    return FALSE;

  struct XmlNode* accessor = xml_node_get_child(technique_common, "accessor");
  if (accessor == NULL)
    return FALSE;

  i32 count = atoi(xml_node_get_attribute(accessor, "count"));
  i32 stride = atoi(xml_node_get_attribute(accessor, "stride"));
  if (stride <= 0 || stride > 4)
    return FALSE;

  char* raw_data = _strdup(xml_node_get_data(tex_coords_data));
  if (raw_data == NULL)
    return FALSE;

  char* next_token = NULL;
  char* raw_part = strtok_s(raw_data, " \t\r\n", &next_token);

  for (i32 tex_coord_num = 0; tex_coord_num < count; tex_coord_num++) {
    vec4 tex_coord = {0};

    for (i32 dim_num = 0; dim_num < stride; dim_num++) {
      if (raw_part == NULL) {
        free(raw_data);
        return FALSE;
      }

      r32 v = (r32)atof(raw_part);
      tex_coord.data[dim_num] = (inverted_y && dim_num == 1) ? (1.0f - v) : v;
      raw_part = strtok_s(NULL, " \t\r\n", &next_token);
    }

    vector_push_back(model_data->tex_coords, &tex_coord);
  }

  free(raw_data);
  return TRUE;
}

void geometry_loader_read_colors(struct ModelData* model_data, struct XmlNode* mesh_data) {
  struct XmlNode* material_node = xml_node_get_child(mesh_data, "polylist");
  if (material_node == NULL)
    material_node = xml_node_get_child(mesh_data, "triangles");
  // if (xml_node_get_child_with_attribute(material_node, "input", "semantic", "COLOR") == NULL)
  //   material_node = xml_node_get_child(mesh_data, "vertices");

  struct XmlNode* colors_location = xml_node_get_child_with_attribute(material_node, "input", "semantic", "COLOR");
  if (colors_location == NULL) {
    vec3 color = (vec3){.r = 1.0f, .g = 1.0f, .b = 1.0f};
    vector_push_back(model_data->colors, &color);
    return;
  }
  char* colors_id = xml_node_get_attribute(colors_location, "source") + 1;
  struct XmlNode* colors_data = xml_node_get_child(xml_node_get_child_with_attribute(mesh_data, "source", "id", colors_id), "float_array");
  // i32 count = atoi(xml_node_get_attribute(colors_data, "count"));
  i32 stride = atoi(xml_node_get_attribute(xml_node_get_child(xml_node_get_child(xml_node_get_child_with_attribute(mesh_data, "source", "id", colors_id), "technique_common"), "accessor"), "stride"));

  char* raw_data = _strdup(xml_node_get_data(colors_data));
  char* next_token = NULL;
  char* raw_part = strtok_s(raw_data, " ", &next_token);
  while (raw_part != NULL) {
    vec4 color = {0};
    for (size_t type_num = 0; type_num < (size_t)stride; type_num++) {
      switch (type_num) {
        case 0: {
          color.r = (r32)atof(raw_part);
          break;
        }
        case 1: {
          color.g = (r32)atof(raw_part);
          break;
        }
        case 2: {
          color.b = (r32)atof(raw_part);
          break;
        }
        case 3: {
          color.a = (r32)atof(raw_part);
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

static void geometry_loader_emit_triangle(struct ModelData* model_data, struct Vector* index_stream, size_t c0, size_t c1, size_t c2, struct GeometryPrimitiveInputs inputs, i32 surface_type) {
  size_t index_count_before = vector_size(model_data->indices);

  geometry_loader_emit_corner(model_data, index_stream, c0, inputs);
  geometry_loader_emit_corner(model_data, index_stream, c1, inputs);
  geometry_loader_emit_corner(model_data, index_stream, c2, inputs);

  if (vector_size(model_data->indices) == index_count_before + 3) {
    vector_push_back(model_data->surface_types, &surface_type);
  }
}

static void geometry_loader_assemble_primitive(struct ModelData* model_data, struct XmlNode* primitive, b8 is_polylist) {
  struct GeometryPrimitiveInputs inputs = geometry_loader_get_primitive_inputs(primitive);
  if (inputs.vertex_offset < 0 || inputs.stride <= 0)
    return;

  struct XmlNode* index_data = xml_node_get_child(primitive, "p");
  if (index_data == NULL)
    return;

  i32 surface_type = geometry_loader_surface_type_from_primitive(primitive);

  struct Vector* index_stream = geometry_loader_parse_i32_list(xml_node_get_data(index_data));

  if (is_polylist) {
    struct XmlNode* vcount_node = xml_node_get_child(primitive, "vcount");

    if (vcount_node != NULL) {
      struct Vector* vcounts = geometry_loader_parse_i32_list(xml_node_get_data(vcount_node));
      size_t corner_cursor = 0;

      for (size_t polygon_num = 0; polygon_num < vector_size(vcounts); polygon_num++) {
        i32 vertex_count = *(i32*)vector_get(vcounts, polygon_num);

        if (vertex_count >= 3) {
          for (size_t tri = 0; tri < (size_t)vertex_count - 2; tri++) {
            geometry_loader_emit_triangle(
                model_data,
                index_stream,
                corner_cursor + 0,
                corner_cursor + tri + 1,
                corner_cursor + tri + 2,
                inputs,
                surface_type);
          }
        }

        corner_cursor += (size_t)vertex_count;
      }

      vector_delete(vcounts);
      free(vcounts);
    }
  } else {
    size_t corner_count = vector_size(index_stream) / (size_t)inputs.stride;

    for (size_t corner_index = 0; corner_index + 2 < corner_count; corner_index += 3) {
      geometry_loader_emit_triangle(
          model_data,
          index_stream,
          corner_index + 0,
          corner_index + 1,
          corner_index + 2,
          inputs,
          surface_type);
    }
  }

  vector_delete(index_stream);
  free(index_stream);
}

void geometry_loader_assemble_vertices(struct ModelData* model_data, struct XmlNode* mesh_data, b8 inverted_y) {
  struct ArrayList* polylists = xml_node_get_children(mesh_data, "polylist");

  if (polylists != NULL) {
    for (size_t i = 0; i < array_list_size(polylists); i++) {
      struct XmlNode* primitive = (struct XmlNode*)array_list_get(polylists, i);
      geometry_loader_assemble_primitive(model_data, primitive, TRUE);
    }
  }

  struct ArrayList* triangles = xml_node_get_children(mesh_data, "triangles");

  if (triangles != NULL) {
    for (size_t i = 0; i < array_list_size(triangles); i++) {
      struct XmlNode* primitive = (struct XmlNode*)array_list_get(triangles, i);
      geometry_loader_assemble_primitive(model_data, primitive, FALSE);
    }
  }

  if (inverted_y == FALSE) {
    for (size_t i = 0; i + 2 < vector_size(model_data->indices); i += 3) {
      i32* index1 = (i32*)vector_get(model_data->indices, i + 1);
      i32* index2 = (i32*)vector_get(model_data->indices, i + 2);

      i32 temp = *index1;
      *index1 = *index2;
      *index2 = temp;
    }
  }
}

void geometry_loader_process_vertex(struct ModelData* model_data, i32 position_index, i32 normal_index, i32 tex_coord_index, i32 color_index) {
  if (position_index < 0 || (size_t)position_index >= vector_size(model_data->vertices)) {
    printf("ERROR: position_index out of range: %d, vertices=%zu\n", position_index, vector_size(model_data->vertices));
    return;
  }

  struct RawVertexModel* current_vertex =
      (struct RawVertexModel*)vector_get(model_data->vertices, (size_t)position_index);

  if (raw_vertex_model_is_set(current_vertex) == FALSE) {
    current_vertex->texture_index = tex_coord_index;
    current_vertex->normal_index = normal_index;
    current_vertex->color_index = color_index;
    vector_push_back(model_data->indices, &position_index);
  } else {
    geometry_loader_deal_with_already_processed_vertex(
        model_data,
        current_vertex,
        tex_coord_index,
        normal_index,
        color_index);
  }
}

r32 geometry_loader_convert_data_to_arrays(struct ModelData* model_data, struct Mesh* model_mesh, b8 animated, b8 inverted_y) {
  r32 furthest_point = 0.0f;

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

    // Normal
    if (current_vertex->normal_index >= 0 && (size_t)current_vertex->normal_index < vector_size(model_data->normals)) {
      vec3* normal_ptr = (vec3*)vector_get(model_data->normals, (size_t)current_vertex->normal_index);
      if (normal_ptr != NULL)
        model_normal = *normal_ptr;
    }

    // Color
    if (current_vertex->color_index >= 0 && (size_t)current_vertex->color_index < vector_size(model_data->colors)) {
      vec3* color_ptr = (vec3*)vector_get(model_data->colors, (size_t)current_vertex->color_index);
      if (color_ptr != NULL)
        model_color = *color_ptr;
    }

    // Texture coordinates
    if (current_vertex->texture_index >= 0 && (size_t)current_vertex->texture_index < vector_size(model_data->tex_coords)) {
      vec2* tex_ptr = (vec2*)vector_get(model_data->tex_coords, (size_t)current_vertex->texture_index);
      if (tex_ptr != NULL)
        model_tex_coord = *tex_ptr;
    }

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

void geometry_loader_deal_with_already_processed_vertex(
    struct ModelData* model_data,
    struct RawVertexModel* previous_vertex,
    i32 new_texture_index,
    i32 new_normal_index,
    i32 new_color_index) {
  if (geometry_loader_vertex_matches(previous_vertex, new_texture_index, new_normal_index, new_color_index)) {
    i32 index = (i32)previous_vertex->index;
    vector_push_back(model_data->indices, &index);
    return;
  }

  struct RawVertexModel duplicate_vertex = {0};

  raw_vertex_model_init(
      &duplicate_vertex,
      (u32)vector_size(model_data->vertices),
      previous_vertex->position,
      previous_vertex->weights_data);

  duplicate_vertex.texture_index = new_texture_index;
  duplicate_vertex.normal_index = new_normal_index;
  duplicate_vertex.color_index = new_color_index;

  i32 duplicate_index = (i32)duplicate_vertex.index;

  vector_push_back(model_data->vertices, &duplicate_vertex);
  vector_push_back(model_data->indices, &duplicate_index);
}

void geometry_loader_remove_unused_vertices(struct ModelData* model_data) {
  for (size_t vertex_num = 0; vertex_num < vector_size(model_data->vertices); vertex_num++) {
    struct RawVertexModel* vertex = (struct RawVertexModel*)vector_get(model_data->vertices, vertex_num);
    if (raw_vertex_model_is_set(vertex) == FALSE) {
      vertex->texture_index = 0;
      vertex->normal_index = 0;
    }
  }
}
