#include "mana/graphics/utilities/collada/modelskinning.h"

void vertex_skin_data_init(struct VertexSkinData* vertex_skin_data) {
  vertex_skin_data->joint_ids = (struct Vector*)malloc(sizeof(struct Vector));
  vertex_skin_data->weights = (struct Vector*)malloc(sizeof(struct Vector));

  vector_init(vertex_skin_data->joint_ids, sizeof(i32));
  vector_init(vertex_skin_data->weights, sizeof(r32));
}

void vertex_skin_data_delete(struct VertexSkinData* vertex_skin_data) {
}

void vertex_skin_data_add_joint_effect(struct VertexSkinData* vertex_skin_data, i32 joint_id, r32 weight) {
  for (size_t weight_num = 0; weight_num < vector_size(vertex_skin_data->weights); weight_num++) {
    if (weight > *(r32*)vector_get(vertex_skin_data->weights, weight_num)) {
      vector_insert(vertex_skin_data->joint_ids, weight_num, &joint_id);
      vector_insert(vertex_skin_data->weights, weight_num, &weight);
      return;
    }
  }
  vector_push_back(vertex_skin_data->joint_ids, &joint_id);
  vector_push_back(vertex_skin_data->weights, &weight);
}

void vertex_skin_data_limit_joint_number(struct VertexSkinData* vertex_skin_data, const i32 max) {
  if (vector_size(vertex_skin_data->joint_ids) > (size_t)max) {
    // TODO: This value should probably be defined
    // r32 top_weights[max];
    r32* top_weights = (r32*)calloc((size_t)max, sizeof(r32));
    r32 total = vertex_skin_data_save_top_weights(vertex_skin_data, top_weights, max);
    vertex_skin_data_refill_weight_list(vertex_skin_data, top_weights, max, total);
    vertex_skin_data_remove_excess_joint_ids(vertex_skin_data, max);
  } else if (vector_size(vertex_skin_data->joint_ids) < (size_t)max)
    vertex_skin_data_fill_empty_weights(vertex_skin_data, max);
}

void vertex_skin_data_fill_empty_weights(struct VertexSkinData* vertex_skin_data, i32 max) {
  i32 zero_int = 0;
  r32 zero_float = 0.0f;
  while (vector_size(vertex_skin_data->joint_ids) < (size_t)max) {
    vector_push_back(vertex_skin_data->joint_ids, &zero_int);
    vector_push_back(vertex_skin_data->weights, &zero_float);
  }
}

r32 vertex_skin_data_save_top_weights(struct VertexSkinData* vertex_skin_data, r32* top_weights, i32 max) {
  r32 total = 0.0f;
  for (size_t top_weight_num = 0; top_weight_num < (size_t)max; top_weight_num++) {
    top_weights[top_weight_num] = *(r32*)vector_get(vertex_skin_data->weights, top_weight_num);
    total += top_weights[top_weight_num];
  }
  return total;
}

void vertex_skin_data_refill_weight_list(struct VertexSkinData* vertex_skin_data, r32* top_weights, i32 max, r32 total) {
  vector_clear(vertex_skin_data->weights);
  for (size_t top_weight_num = 0; top_weight_num < (size_t)max; top_weight_num++) {
    r32 new_weight = min(top_weights[top_weight_num] / total, 1.0f);
    vector_push_back(vertex_skin_data->weights, &new_weight);
  }
}

void vertex_skin_data_remove_excess_joint_ids(struct VertexSkinData* vertex_skin_data, i32 max) {
  while (vector_size(vertex_skin_data->joint_ids) > (size_t)max)
    vector_remove(vertex_skin_data->joint_ids, vector_size(vertex_skin_data->joint_ids) - 1);
}

struct SkinningData* skin_loader_extract_skin_data(struct XmlNode* skinning_data, i32 max_weights) {
  skinning_data = xml_node_get_child(xml_node_get_child(skinning_data, "controller"), "skin");
  struct Vector* joints_list = skin_loader_load_joints_list(skinning_data);
  struct Vector* weights = skin_loader_load_weights(skinning_data);
  struct XmlNode* weights_data_node = xml_node_get_child(skinning_data, "vertex_weights");
  struct Vector* effector_joint_counts = skin_loader_get_effective_joints_counts(weights_data_node);
  struct Vector* vertex_weights = skin_loader_get_skin_data(max_weights, weights_data_node, effector_joint_counts, weights);
  struct SkinningData* parsed_skinning_data = (struct SkinningData*)malloc(sizeof(struct SkinningData));
  skinning_data_init(parsed_skinning_data, joints_list, vertex_weights);
  vector_delete(weights);
  free(weights);
  vector_delete(effector_joint_counts);
  free(effector_joint_counts);
  return parsed_skinning_data;
}

struct Vector* skin_loader_load_joints_list(struct XmlNode* skinning_data) {
  struct XmlNode* input_node = xml_node_get_child(skinning_data, "vertex_weights");
  char* joint_data_id = xml_node_get_attribute(xml_node_get_child_with_attribute(input_node, "input", "semantic", "JOINT"), "source") + 1;
  struct XmlNode* joint_node = xml_node_get_child(xml_node_get_child_with_attribute(skinning_data, "source", "id", joint_data_id), "Name_array");

  char* raw_data = _strdup(xml_node_get_data(joint_node));
  struct Vector* joints_list = (struct Vector*)malloc(sizeof(struct Vector));
  vector_init(joints_list, sizeof(char*));
  char* next_token = NULL;
  char* raw_part = strtok_s(raw_data, " ", &next_token);
  while (raw_part != NULL) {
    // TODO: These will need to be freed
    char* parsed_joint = _strdup(raw_part);
    vector_push_back(joints_list, &parsed_joint);
    raw_part = strtok_s(NULL, " ", &next_token);
  }
  free(raw_data);
  return joints_list;
}

struct Vector* skin_loader_load_weights(struct XmlNode* skinning_data) {
  struct XmlNode* input_node = xml_node_get_child(skinning_data, "vertex_weights");
  char* weights_data_id = xml_node_get_attribute(xml_node_get_child_with_attribute(input_node, "input", "semantic", "WEIGHT"), "source") + 1;
  struct XmlNode* weights_node = xml_node_get_child(xml_node_get_child_with_attribute(skinning_data, "source", "id", weights_data_id), "float_array");

  char* raw_data = _strdup(xml_node_get_data(weights_node));
  struct Vector* weights = (struct Vector*)malloc(sizeof(struct Vector));
  vector_init(weights, sizeof(r32));
  char* next_token = NULL;
  char* raw_part = strtok_s(raw_data, " ", &next_token);
  while (raw_part != NULL) {
    r32 parsed_weight = (r32)atof(raw_part);
    vector_push_back(weights, &parsed_weight);
    raw_part = strtok_s(NULL, " ", &next_token);
  }
  free(raw_data);
  return weights;
}

struct Vector* skin_loader_get_effective_joints_counts(struct XmlNode* weights_data_node) {
  char* raw_data = _strdup(xml_node_get_data(xml_node_get_child(weights_data_node, "vcount")));
  struct Vector* counts = (struct Vector*)malloc(sizeof(struct Vector));
  vector_init(counts, sizeof(i32));
  char* next_token = NULL;
  char* raw_part = strtok_s(raw_data, " ", &next_token);
  while (raw_part != NULL) {
    i32 parsed_count = atoi(raw_part);
    vector_push_back(counts, &parsed_count);
    raw_part = strtok_s(NULL, " ", &next_token);
  }
  free(raw_data);
  return counts;
}

struct Vector* skin_loader_get_skin_data(i32 max_weights, struct XmlNode* weights_data_node, struct Vector* counts, struct Vector* weights) {
  char* raw_data = _strdup(xml_node_get_data(xml_node_get_child(weights_data_node, "v")));
  struct Vector* skinning_data = (struct Vector*)malloc(sizeof(struct Vector));
  vector_init(skinning_data, sizeof(struct VertexSkinData));
  char* next_token = NULL;
  char* raw_part = strtok_s(raw_data, " ", &next_token);
  for (size_t count = 0; count < vector_size(counts); count++) {
    struct VertexSkinData skin_data = {0};
    vertex_skin_data_init(&skin_data);
    for (size_t loop_num = 0; loop_num < (size_t)(*(i32*)vector_get(counts, count)); loop_num++) {
      i32 joint_id = atoi(raw_part);
      raw_part = strtok_s(NULL, " ", &next_token);
      i32 weight_id = atoi(raw_part);
      raw_part = strtok_s(NULL, " ", &next_token);
      // printf("Count: %d\n", parsed_count);
      vertex_skin_data_add_joint_effect(&skin_data, joint_id, *(r32*)vector_get(weights, (size_t)weight_id));
    }
    vertex_skin_data_limit_joint_number(&skin_data, max_weights);
    vector_push_back(skinning_data, &skin_data);
  }
  free(raw_data);
  return skinning_data;
}
