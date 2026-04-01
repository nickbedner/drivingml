#pragma once

#include <mana/math/advmath.h>
#include <mana/storage/storage.h>

#include "mana/core/corecommon.h"
#include "mana/graphics/entities/model/modelcommon.h"
#include "mana/graphics/utilities/mesh/mesh.h"
#include "mana/utilities/xmlparser.h"

void vertex_skin_data_init(struct VertexSkinData* vertex_skin_data);
void vertex_skin_data_delete(struct VertexSkinData* vertex_skin_data);
void vertex_skin_data_add_joint_effect(struct VertexSkinData* vertex_skin_data, i32 joint_id, r32 weight);
void vertex_skin_data_limit_joint_number(struct VertexSkinData* vertex_skin_data, const i32 max);
void vertex_skin_data_fill_empty_weights(struct VertexSkinData* vertex_skin_data, i32 max);
r32 vertex_skin_data_save_top_weights(struct VertexSkinData* vertex_skin_data, r32* top_weights, i32 max);
void vertex_skin_data_refill_weight_list(struct VertexSkinData* vertex_skin_data, r32* top_weights, i32 max, r32 total);
void vertex_skin_data_remove_excess_joint_ids(struct VertexSkinData* vertex_skin_data, i32 max);

internal inline void skinning_data_init(struct SkinningData* skinning_data, struct Vector* joint_order, struct Vector* vertices_skin_data) {
  skinning_data->joint_order = joint_order;
  skinning_data->vertices_skin_data = vertices_skin_data;
}

struct SkinningData* skin_loader_extract_skin_data(struct XmlNode* skinning_data, i32 max_weights);
struct Vector* skin_loader_load_joints_list(struct XmlNode* skinning_data);
struct Vector* skin_loader_load_weights(struct XmlNode* skinning_data);
struct Vector* skin_loader_get_effective_joints_counts(struct XmlNode* weights_data_node);
struct Vector* skin_loader_get_skin_data(i32 max_weights, struct XmlNode* weights_data_node, struct Vector* counts, struct Vector* weights);
