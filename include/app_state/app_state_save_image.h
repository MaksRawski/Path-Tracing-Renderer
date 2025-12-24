#ifndef APP_STATE_SAVE_IMAGE_H_
#define APP_STATE_SAVE_IMAGE_H_

#include "glad/gl.h"
#include "opengl/resolution.h"

typedef struct {
  char path[1024];
  bool to_save;
} AppStateSaveImageInfo;

AppStateSaveImageInfo AppStateSaveImageInfo_default(void);
void AppState_save_image(AppStateSaveImageInfo *save_image_info, GLuint fbo,
                         OpenGLResolution resolution);

#endif // APP_STATE_SAVE_IMAGE_H_
