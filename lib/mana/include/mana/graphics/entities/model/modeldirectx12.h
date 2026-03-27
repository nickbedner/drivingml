#pragma once

#include "mana/graphics/entities/model/modelcommon.h"
#include "mana/graphics/utilities/collada/modelanimation.h"
#include "mana/graphics/utilities/collada/modelanimator.h"
#include "mana/graphics/utilities/collada/modelgeometry.h"
#include "mana/graphics/utilities/collada/modelskeleton.h"
#include "mana/graphics/utilities/collada/modelskinning.h"

void model_directx_12_clone_init(struct ModelCommon* model_common, struct APICommon* api_common);
void model_directx_12_clone_delete(struct ModelCommon* model_common, struct APICommon* api_common);
void model_directx_12_render(struct ModelCommon* model_common, struct GBuffer* gbuffer, double delta_time);
void model_directx_12_update_uniforms(struct ModelCommon* model_common, struct APICommon* api_common, struct GBuffer* gbuffer, vec3d position, vec3 light_pos, vec3 diffuse_color, vec3 ambient_color, vec3 specular_light);
