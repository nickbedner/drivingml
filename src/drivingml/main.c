#ifdef __ANDROID__
#include <jni.h>
#endif

#include <mana/core/corecommon.h>

#include "drivingml/drivingml.h"

int main(int argc, char* argv[]) {
  log_message(LOG_SEVERITY_INFO, "Starting Driving ML...\n");

  struct DrivingML drivingml = {0};
  const uint_fast8_t drivingml_error = drivingml_init(&drivingml);

  switch (drivingml_error) {
    case (DRIVINGML_SUCCESS): {
      break;
    }
    case (DRIVINGML_MANA_ERROR): {
      log_message(LOG_SEVERITY_ERROR, "Failed to setup Mana for Project DrivingML!\n");
      return 1;
    }
    case (DRIVINGML_WINDOW_ERROR): {
      log_message(LOG_SEVERITY_ERROR, "Failed to setup the window for Project DrivingML!\n");
      return 2;
    }
    default: {
      log_message(LOG_SEVERITY_CRITICAL, "Unknown DrivingML error! Error code: %d\n", drivingml_error);
      return 3;
    }
  }

  drivingml_start(&drivingml);
  drivingml_delete(&drivingml);

  return 0;
}
