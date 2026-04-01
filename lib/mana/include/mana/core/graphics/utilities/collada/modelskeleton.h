#pragma once

#include <mana/core/math/advmath.h>
#include <mana/core/storage/storage.h>

#include "mana/core/corecommon.h"
#include "mana/core/graphics/entities/model/modelcommon.h"
#include "mana/core/graphics/utilities/mesh/mesh.h"
#include "mana/core/utilities/xmlparser.h"

internal inline void joint_data_init(struct JointData* joint_data, i32 index, char* name_id, mat4 bind_local_transform) {
  joint_data->index = index;
  joint_data->name_id = _strdup(name_id);
  joint_data->bind_local_transform = bind_local_transform;
  joint_data->children = (struct ArrayList*)malloc(sizeof(struct ArrayList));
  array_list_init(joint_data->children);
}

internal inline void skeleton_data_init(struct SkeletonData* skeleton_data, i32 joint_count, struct JointData* head_joint) {
  skeleton_data->joint_count = joint_count;
  skeleton_data->head_joint = head_joint;
}

struct SkeletonData* skeleton_loader_extract_bone_data(struct XmlNode* visual_scene_node, struct Vector* bone_order, b8 inverted_y);
struct JointData* skeleton_loader_load_joint_data(struct XmlNode* joint_node, struct Vector* bone_order, b8 is_root, i32* joint_count, b8 inverted_y);
struct JointData* skeleton_loader_extract_main_joint_data(struct XmlNode* joint_node, struct Vector* bone_order, b8 is_root, i32* joint_count, b8 inverted_y);
mat4 skeleton_loader_convert_data(char* matrix_data);
