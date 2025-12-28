#ifndef APP_STATE_SAVE_IMAGE_H_
#define APP_STATE_SAVE_IMAGE_H_

#include "glad/gl.h"
#include <stdbool.h>

typedef struct {
  char path[1024];
  bool to_save;
} AppStateSaveImageInfo;

AppStateSaveImageInfo AppStateSaveImageInfo_default(void);

#endif // APP_STATE_SAVE_IMAGE_H_
