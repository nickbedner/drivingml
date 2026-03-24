#pragma once

#include <mana/math/advmath.h>
#include <mana/storage/storage.h>

#include "mana/core/corecommon.h"
#include "mana/graphics/entities/model/modelcommon.h"
#include "mana/graphics/utilities/mesh/mesh.h"
#include "mana/utilities/xmlparser.h"

static inline void joint_data_init(struct JointData* joint_data, int32_t index, char* name_id, mat4 bind_local_transform) {
  joint_data->index = index;
  joint_data->name_id = _strdup(name_id);
  joint_data->bind_local_transform = bind_local_transform;
  joint_data->children = (struct ArrayList*)malloc(sizeof(struct ArrayList));
  array_list_init(joint_data->children);
}

static inline void skeleton_data_init(struct SkeletonData* skeleton_data, int32_t joint_count, struct JointData* head_joint) {
  skeleton_data->joint_count = joint_count;
  skeleton_data->head_joint = head_joint;
}

struct SkeletonData* skeleton_loader_extract_bone_data(struct XmlNode* visual_scene_node, struct Vector* bone_order, bool inverted_y);
struct JointData* skeleton_loader_load_joint_data(struct XmlNode* joint_node, struct Vector* bone_order, bool is_root, int32_t* joint_count, bool inverted_y);
struct JointData* skeleton_loader_extract_main_joint_data(struct XmlNode* joint_node, struct Vector* bone_order, bool is_root, int32_t* joint_count, bool inverted_y);
mat4 skeleton_loader_convert_data(char* matrix_data);
