#pragma once

#include "mana/core/graphics/entities/model/modelcommon.h"
#include "mana/core/graphics/utilities/collada/modelanimation.h"
#include "mana/core/graphics/utilities/collada/modelanimator.h"
#include "mana/core/graphics/utilities/collada/modelgeometry.h"
#include "mana/core/graphics/utilities/collada/modelskeleton.h"
#include "mana/core/graphics/utilities/collada/modelskinning.h"

void model_directx_12_clone_init(struct ModelCommon* model_common, struct APICommon* api_common);
void model_directx_12_clone_delete(struct ModelCommon* model_common, struct APICommon* api_common);
void model_directx_12_render(struct ModelCommon* model_common, struct GBuffer* gbuffer, r64 delta_time);
void model_directx_12_update_uniforms(struct ModelCommon* model_common, struct APICommon* api_common, struct GBuffer* gbuffer, vec3d position, vec4 light_pos, vec4 diffuse_color, vec4 ambient_color, vec4 specular_light);
