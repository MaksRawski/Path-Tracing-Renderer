#ifndef APP_STATE_SAVE_IMAGE_H_
#define APP_STATE_SAVE_IMAGE_H_

#include "const_string.h"
#include "glad/gl.h"
#include "opengl/resolution.h"

typedef struct {
  ConstString path;
  bool to_save;
} AppStateSaveImageInfo;

void AppState_save_image(AppStateSaveImageInfo *save_image_info, GLuint fbo,
                         OpenGLResolution resolution);

#endif // APP_STATE_SAVE_IMAGE_H_
