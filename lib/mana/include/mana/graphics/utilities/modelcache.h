#pragma once

#include <mana/graphics/apis/api.h>
#include <stdarg.h>

#include "mana/graphics/entities/model/model.h"
#include "mana/graphics/graphicscommon.h"
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
};

void model_cache_init(struct ModelCache *model_cache);
void model_cache_delete(struct ModelCache *model_cache, struct APICommon *api_common);
// void model_cache_add(struct ModelCache *model_cache, struct APICommon *api_common, size_t n_models, ...);
void model_cache_add(struct ModelCache *model_cache, struct APICommon *api_common, struct ModelSettings *model_settings, size_t num);
struct Model *model_cache_get(struct ModelCache *model_cache, struct APICommon *api_common, char *model_name);
