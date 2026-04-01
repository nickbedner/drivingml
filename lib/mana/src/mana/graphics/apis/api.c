#include "mana/graphics/apis/api.h"

u8 gpu_api_init(struct API* api, enum API_TYPE preferred_api_type) {
  // TODO: Assign rendering struct here?

  if (api_handlers[preferred_api_type].api_func.api_init) {
    if (api_handlers[preferred_api_type].api_func.api_init(&(api->api_common)) == API_SUCCESS) {
      api->api_func = api_handlers[preferred_api_type].api_func;
      api->api_common.api_type = api_handlers[preferred_api_type].api_type;
      api->api_common.inverted_y = api_handlers[preferred_api_type].inverted_y;
      return API_SUCCESS;
    }
  }

  // Note: If preferred API was set and failed try the others for fallback
  for (size_t api_num = 1; api_num < API_LAST; api_num++) {
    if (api_handlers[api_num].api_type == preferred_api_type)
      continue;

    if (api_handlers[api_num].api_func.api_init) {
      if (api_handlers[api_num].api_func.api_init(&(api->api_common)) == API_SUCCESS) {
        api->api_common.api_type = api_handlers[api_num].api_type;
        api->api_common.inverted_y = api_handlers[api_num].inverted_y;
        return API_SUCCESS;
      }
    }
  }

  return API_ERROR;
}

void gpu_api_delete(struct API* api) {
  // Note: Check to make sure delete function exists before calling it
  if (api->api_func.api_delete)
    api->api_func.api_delete(&api->api_common);
}
