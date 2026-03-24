#include "mana/graphics/utilities/modelcache.h"

void model_cache_init(struct ModelCache* model_cache) {
  // Note: Store as references because it would be dangerous to realloc in linear memory
  map_init(&model_cache->models, sizeof(struct Model*));
}

void model_cache_delete(struct ModelCache* model_cache, struct APICommon* api_common) {
  const char* model_key;
  struct MapIter model_iter = map_iter();
  while ((model_key = map_next(&model_cache->models, &model_iter))) {
    struct Model* model = *(struct Model**)map_get(&model_cache->models, model_key);
    model_delete(model, api_common);
    free(model);
  }

  map_delete(&model_cache->models);
}

// TODO: Maybe allow for init from structs instead out outside
void model_cache_add(struct ModelCache* model_cache, struct APICommon* api_common, struct ModelSettings* model_settings, size_t num) {
  struct Model* model = (struct Model*)malloc(sizeof(struct Model));
  model_init(model, api_common, model_settings, num);
  map_set(&(model_cache->models), model->model_common.path, &model);  // Store full path in case of models having same texture name like diffuse
}

//// TODO: Maybe allow for init from structs instead out outside
// void model_cache_add(struct ModelCache *model_cache, struct APICommon *api_common, size_t n_models, ...) {
//   va_list args;
//   va_start(args, n_models);
//
//   while (n_models-- > 0) {
//     struct ModelSettings model_settings = va_arg(args, struct ModelSettings);
//     struct Model *model = malloc(sizeof(struct Model));
//     model_init(model, api_common, model_settings);
//     map_set(&(model_cache->models), model->model_common.path, &model); // Store full path in case of models having same texture name like diffuse
//   }
//
//   va_end(args);
// }

struct Model* model_cache_get(struct ModelCache* model_cache, struct APICommon* api_common, char* model_name) {
  return model_get_clone(*((struct Model**)map_get(&(model_cache->models), model_name)), api_common);
}

// TODO: Maybe get instanced/batched where specific vertex data is not needed
