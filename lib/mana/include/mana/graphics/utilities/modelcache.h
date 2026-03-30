#pragma once

#include <stdarg.h>

#include "mana/graphics/apis/api.h"
#include "mana/graphics/entities/model/model.h"
#include "mana/graphics/graphicscommon.h"
#include "mana/graphics/shaders/modelshader.h"
#include "mana/graphics/shaders/modelstaticshader.h"
#include "mana/graphics/shaders/shader.h"
#include "mana/graphics/utilities/collada/modelanimation.h"
#include "mana/graphics/utilities/collada/modelanimator.h"
#include "mana/graphics/utilities/collada/modelgeometry.h"
#include "mana/graphics/utilities/collada/modelskeleton.h"
#include "mana/graphics/utilities/collada/modelskinning.h"
#include "mana/graphics/utilities/mesh/mesh.h"
#include "mana/graphics/utilities/texture/texture.h"
#include "mana/utilities/xmlparser.h"

struct APICommon;

struct ModelCache {
  struct Map models;
  struct ModelShader model_shader;
  struct ModelStaticShader model_static_shader;
  VkDescriptorSet* model_descriptor_set;
  VkDescriptorSet* model_static_descriptor_set;
};

void model_cache_init(struct ModelCache* model_cache, struct APICommon* api_common, uint32_t width, uint32_t height, uint_fast8_t supersample_scale, struct GBufferCommon* gbuffer_common, uint_fast8_t msaa_samples, uint_fast32_t descriptors);
void model_cache_delete(struct ModelCache* model_cache, struct APICommon* api_common);
// void model_cache_add(struct ModelCache *model_cache, struct APICommon *api_common, size_t n_models, ...);
void model_cache_add(struct ModelCache* model_cache, struct APICommon* api_common, struct ModelSettings* model_settings, size_t num, bool animated);
struct Model* model_cache_get(struct ModelCache* model_cache, struct APICommon* api_common, char* model_name);
