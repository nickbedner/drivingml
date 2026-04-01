#pragma once

#include "mana/graphics/utilities/mesh/meshcommon.h"

u8 mesh_vulkan_init(struct MeshCommon* mesh_common, enum MESH_TYPE mesh_type, struct APICommon* api_common);
void mesh_vulkan_delete(struct MeshCommon* mesh_common, struct APICommon* api_common);
void mesh_vulkan_generate_buffers(struct MeshCommon* mesh_common, struct APICommon* api_common);
