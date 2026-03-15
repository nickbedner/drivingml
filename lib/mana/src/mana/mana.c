#include "mana/mana.h"

uint_fast8_t mana_init(struct Mana* mana, enum API_TYPE preferred_api_type) {
  mana_search_for_assets_directory(mana->api.api_common.asset_directory, sizeof(mana->api.api_common.asset_directory));

  // TODO: If preferred doesn't exist or not set yet just use platform preferred, still doesn't work then cycle through them all and if nothing quit
  const uint_fast8_t gpu_api_error = gpu_api_init(&(mana->api), preferred_api_type);
  switch (gpu_api_error) {
    case (API_SUCCESS): {
      break;
    }
    case (API_ERROR): {
      log_message(LOG_SEVERITY_ERROR, "Failed to setup gpu api for Mana!\n");
      return MANA_GPU_API_ERROR;
    }
    default: {
      log_message(LOG_SEVERITY_CRITICAL, "Unknown mana error! Error code: %d\n", gpu_api_error);
      return MANA_LAST_ERROR;
    }
  }

  return MANA_SUCCESS;
}

void mana_delete(struct Mana* mana) {
  gpu_api_delete(&(mana->api));
}
