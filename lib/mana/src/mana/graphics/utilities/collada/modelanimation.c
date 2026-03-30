#include "mana/graphics/utilities/collada/modelanimation.h"

struct AnimationData* animation_extract_animation(struct XmlNode* animation_data, struct XmlNode* joint_hierarchy, bool inverted_y) {
  char* root_node = NULL;
  float duration = 0.0f;
  struct Vector* times = NULL;
  struct ArrayList* key_frames = NULL;
  struct ArrayList* animation_nodes = NULL;
  struct AnimationData* anim_data = NULL;

  if (animation_data == NULL || joint_hierarchy == NULL)
    return NULL;

  /* Find root joint name safely */
  struct XmlNode* visual_scene = xml_node_get_child(joint_hierarchy, "visual_scene");
  if (visual_scene == NULL)
    return NULL;

  struct XmlNode* armature_node = xml_node_get_child_with_attribute(visual_scene, "node", "id", "Armature");

  struct XmlNode* root_joint_node = NULL;
  if (armature_node != NULL)
    root_joint_node = xml_node_get_child(armature_node, "node");

  /* Fallback: use first node in visual_scene if no Armature wrapper exists */
  if (root_joint_node == NULL)
    root_joint_node = xml_node_get_child(visual_scene, "node");

  if (root_joint_node != NULL)
    root_node = xml_node_get_attribute(root_joint_node, "id");

  if (root_node == NULL)
    return NULL;

  times = animation_get_key_times(animation_data);
  if (times == NULL)
    return NULL;

  if (vector_size(times) == 0) {
    vector_delete(times);
    free(times);
    return NULL; /* no animation keys in this file */
  }

  duration = *(float*)vector_get(times, vector_size(times) - 1);

  key_frames = animation_init_key_frames(times);

  vector_delete(times);
  free(times);
  times = NULL;

  if (key_frames == NULL)
    return NULL;

  animation_nodes = xml_node_get_children(animation_data, "animation");
  if (animation_nodes != NULL) {
    for (size_t joint_node_num = 0; joint_node_num < array_list_size(animation_nodes); joint_node_num++) {
      struct XmlNode* joint_anim = (struct XmlNode*)array_list_get(animation_nodes, joint_node_num);
      if (joint_anim != NULL)
        animation_load_joint_transform(key_frames, joint_anim, root_node, inverted_y);
    }
  }

  anim_data = (struct AnimationData*)malloc(sizeof(struct AnimationData));
  if (anim_data == NULL) {
    /* cleanup key_frames here if you have a destructor for it */
    return NULL;
  }

  animation_data_init(anim_data, duration, key_frames);
  return anim_data;
}

struct Vector* animation_get_key_times(struct XmlNode* animation_data) {
  struct Vector* times = (struct Vector*)malloc(sizeof(struct Vector));
  if (times == NULL)
    return NULL;

  vector_init(times, sizeof(float));

  if (animation_data == NULL)
    return times;

  /* Some files wrap channels in an inner <animation>, some may already pass one in */
  struct XmlNode* animation_node = xml_node_get_child(animation_data, "animation");
  if (animation_node == NULL)
    animation_node = animation_data;

  if (animation_node == NULL)
    return times;

  /* Best case: find the sampler INPUT source (time keys) */
  struct XmlNode* sampler_node = xml_node_get_child(animation_node, "sampler");
  struct XmlNode* input_node = NULL;
  const char* time_source_attr = NULL;
  const char* time_source_id = NULL;
  struct XmlNode* source_node = NULL;
  struct XmlNode* time_data = NULL;

  if (sampler_node != NULL) {
    input_node = xml_node_get_child_with_attribute(sampler_node, "input", "semantic", "INPUT");
    if (input_node != NULL) {
      time_source_attr = xml_node_get_attribute(input_node, "source");
      if (time_source_attr != NULL && time_source_attr[0] == '#') {
        time_source_id = time_source_attr + 1;
        source_node = xml_node_get_child_with_attribute(animation_node, "source", "id", time_source_id);
      }
    }
  }

  /* Fallback: use the first <source> if no sampler/input path exists */
  if (source_node == NULL)
    source_node = xml_node_get_child(animation_node, "source");

  if (source_node == NULL)
    return times;

  time_data = xml_node_get_child(source_node, "float_array");
  if (time_data == NULL)
    return times;

  const char* data_text = xml_node_get_data(time_data);
  if (data_text == NULL)
    return times;

  char* raw_times = _strdup(data_text);
  if (raw_times == NULL)
    return times;

  char* next_token = NULL;
  char* raw_part = strtok_s(raw_times, " \t\r\n", &next_token);

  while (raw_part != NULL) {
    float time = (float)atof(raw_part);
    vector_push_back(times, &time);
    raw_part = strtok_s(NULL, " \t\r\n", &next_token);
  }

  free(raw_times);
  return times;
}

struct ArrayList* animation_init_key_frames(struct Vector* times) {
  struct ArrayList* frames = (struct ArrayList*)malloc(sizeof(struct ArrayList));
  array_list_init(frames);
  for (size_t frame_num = 0; frame_num < vector_size(times); frame_num++) {
    struct KeyFrameData* key_frame_data = (struct KeyFrameData*)malloc(sizeof(struct KeyFrameData));
    key_frame_data_init(key_frame_data, *(float*)vector_get(times, frame_num));
    array_list_add(frames, key_frame_data);
  }
  return frames;
}

void animation_load_joint_transform(struct ArrayList* frames, struct XmlNode* joint_data, char* root_node_id, bool inverted_y) {
  char* joint_name_raw = xml_node_get_attribute(xml_node_get_child(joint_data, "channel"), "target");
  char* slash_pos = strchr(joint_name_raw, '/');
  size_t joint_name_length = (size_t)(slash_pos - joint_name_raw);
  char* joint_name_id = (char*)malloc(sizeof(char) * (joint_name_length + 1));
  memcpy(joint_name_id, joint_name_raw, joint_name_length);
  joint_name_id[joint_name_length] = '\0';
  char* data_id = xml_node_get_attribute(xml_node_get_child_with_attribute(xml_node_get_child(joint_data, "sampler"), "input", "semantic", "OUTPUT"), "source") + 1;
  // TODO: Support other animation transforms
  if (strstr(data_id, "matrix-output") != NULL) {
    struct XmlNode* transform_data = xml_node_get_child_with_attribute(joint_data, "source", "id", data_id);
    char* raw_data = xml_node_get_data(xml_node_get_child(transform_data, "float_array"));
    animation_process_transforms(joint_name_id, raw_data, frames, strcmp(joint_name_id, root_node_id) == 0, inverted_y);
  }
  free(joint_name_id);
}

void animation_process_transforms(char* joint_name, char* raw_data, struct ArrayList* key_frames, bool root, bool inverted_y) {
  char* next_token = NULL;
  char* raw_part = strtok_s(raw_data, " ", &next_token);
  for (size_t key_frame_num = 0; key_frame_num < array_list_size(key_frames); key_frame_num++) {
    mat4 transform = MAT4_ZERO;

    for (int matrix_value = 0; matrix_value < 16; matrix_value++) {
      *(((float*)&transform) + matrix_value) = (float)atof(raw_part);
      raw_part = strtok_s(NULL, " ", &next_token);
    }

    transform = mat4_transpose(transform);

    if (root) {
      mat4 correction = mat4_rotate(MAT4_IDENTITY, degree_to_radian(-90.0f), (vec3){.data[0] = 1.0f, .data[1] = 0.0f, .data[2] = 0.0f});
      // if (inverted_y == false)
      //   correction.data[10] = -1.0f; // Mirror on the Z-axis
      transform = mat4_mul(correction, transform);
    }

    struct JointTransformData* joint_transform_data = (struct JointTransformData*)malloc(sizeof(struct JointTransformData));
    joint_transform_data_init(joint_transform_data, _strdup(joint_name), transform);
    array_list_add(((struct KeyFrameData*)array_list_get(key_frames, key_frame_num))->joint_transforms, joint_transform_data);
  }
}
