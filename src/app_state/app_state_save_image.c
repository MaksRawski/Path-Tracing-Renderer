#include "app_state/app_state_save_image.h"

AppStateSaveImageInfo AppStateSaveImageInfo_default(void) {
  return (AppStateSaveImageInfo){.path = "output.png", .to_save = false};
}
