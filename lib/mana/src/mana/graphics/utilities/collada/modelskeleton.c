#include "mana/graphics/utilities/collada/modelskeleton.h"

struct SkeletonData* skeleton_loader_extract_bone_data(struct XmlNode* visual_scene_node, struct Vector* bone_order, bool inverted_y) {
  struct XmlNode* armature_data = xml_node_get_child_with_attribute(xml_node_get_child(visual_scene_node, "visual_scene"), "node", "id", "Armature");
  struct XmlNode* head_node = xml_node_get_child(armature_data, "node");

  int32_t joint_count = 0;
  struct JointData* head_joint = skeleton_loader_load_joint_data(head_node, bone_order, true, &joint_count, inverted_y);
  struct SkeletonData* skeleton_data = (struct SkeletonData*)malloc(sizeof(struct SkeletonData));
  skeleton_data_init(skeleton_data, joint_count, head_joint);
  return skeleton_data;
}

struct JointData* skeleton_loader_load_joint_data(struct XmlNode* joint_node, struct Vector* bone_order, bool is_root, int32_t* joint_count, bool inverted_y) {
  struct JointData* joint = skeleton_loader_extract_main_joint_data(joint_node, bone_order, is_root, joint_count, inverted_y);
  struct ArrayList* joint_node_children = xml_node_get_children(joint_node, "node");
  if (joint_node_children != NULL) {
    for (size_t child_num = 0; child_num < array_list_size(joint_node_children); child_num++) {
      struct XmlNode* child_node = (struct XmlNode*)array_list_get(joint_node_children, child_num);
      array_list_add(joint->children, skeleton_loader_load_joint_data(child_node, bone_order, false, joint_count, inverted_y));
    }
  }
  return joint;
}

struct JointData* skeleton_loader_extract_main_joint_data(struct XmlNode* joint_node, struct Vector* bone_order, bool is_root, int32_t* joint_count, bool inverted_y) {
  char* name_id = xml_node_get_attribute(joint_node, "id");
  int32_t index = -1;

  for (size_t bone_num = 0; bone_num < vector_size(bone_order); bone_num++) {
    char** bone_id = (char**)vector_get(bone_order, bone_num);
    if (strcmp(name_id, *bone_id) == 0) {
      index = (int32_t)bone_num;
      break;
    }
  }

  char* matrix_data = xml_node_get_data(xml_node_get_child(joint_node, "matrix"));
  mat4 matrix = skeleton_loader_convert_data(matrix_data);
  matrix = mat4_transpose(matrix);
  if (is_root) {
    mat4 correction = mat4_rotate(MAT4_IDENTITY, degree_to_radian(-90.0f), (vec3){.data[0] = 1.0f, .data[1] = 0.0f, .data[2] = 0.0f});
    // if (inverted_y == false)
    //   correction.data[10] = -1.0f; // Mirror on the Z-axis
    matrix = mat4_mul(correction, matrix);
  }
  (*joint_count)++;
  struct JointData* joint_data = (struct JointData*)malloc(sizeof(struct JointData));
  joint_data_init(joint_data, index, name_id, matrix);
  return joint_data;
}

mat4 skeleton_loader_convert_data(char* matrix_data) {
  mat4 dest = MAT4_ZERO;
  char* raw_data = _strdup(matrix_data);
  char* next_token = NULL;
  char* raw_part = strtok_s(raw_data, " ", &next_token);
  int row = 0;
  for (int matrix_value = 0; matrix_value < 4; matrix_value++) {
    float m0 = (float)atof(raw_part);
    raw_part = strtok_s(NULL, " ", &next_token);
    float m1 = (float)atof(raw_part);
    raw_part = strtok_s(NULL, " ", &next_token);
    float m2 = (float)atof(raw_part);
    raw_part = strtok_s(NULL, " ", &next_token);
    float m3 = (float)atof(raw_part);
    raw_part = strtok_s(NULL, " ", &next_token);
    vec4 matrix_slice = (vec4){.data[0] = m0, .data[1] = m1, .data[2] = m2, .data[3] = m3};
    // dest.vecs[matrix_value] = matrix_slice;
    *((vec4*)&dest.data[row]) = matrix_slice;
    row += 4;
  }
  free(raw_data);
  return dest;
}
