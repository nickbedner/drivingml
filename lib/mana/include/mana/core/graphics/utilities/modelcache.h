#pragma once

#include <stdarg.h>

#include "mana/core/graphics/apis/api.h"
#include "mana/core/graphics/entities/model/model.h"
#include "mana/core/graphics/graphicscommon.h"
#include "mana/core/graphics/shaders/modelshader.h"
#include "mana/core/graphics/shaders/modelstaticshader.h"
#include "mana/core/graphics/shaders/shader.h"
#include "mana/core/graphics/utilities/collada/modelanimation.h"
#include "mana/core/graphics/utilities/collada/modelanimator.h"
#include "mana/core/graphics/utilities/collada/modelgeometry.h"
#include "mana/core/graphics/utilities/collada/modelskeleton.h"
#include "mana/core/graphics/utilities/collada/modelskinning.h"
#include "mana/core/graphics/utilities/mesh/mesh.h"
#include "mana/core/graphics/utilities/texture/texture.h"
#include "mana/core/utilities/xmlparser.h"

struct APICommon;

struct ModelCache {
  struct Map models;
  struct ModelShader model_shader;
  struct ModelStaticShader model_static_shader;
  VkDescriptorSet* model_descriptor_set;
  VkDescriptorSet* model_static_descriptor_set;
};

void model_cache_init(struct ModelCache* model_cache, struct APICommon* api_common, u32 width, u32 height, u8 supersample_scale, struct GBufferCommon* gbuffer_common, u8 msaa_samples, uint_fast32_t descriptors);
void model_cache_delete(struct ModelCache* model_cache, struct APICommon* api_common);
// void model_cache_add(struct ModelCache *model_cache, struct APICommon *api_common, size_t n_models, ...);
void model_cache_add(struct ModelCache* model_cache, struct APICommon* api_common, struct ModelSettings* model_settings, size_t num, b8 animated);
struct Model* model_cache_get(struct ModelCache* model_cache, struct APICommon* api_common, char* model_name);
