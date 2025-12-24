#include "app_state/app_state_save_image.h"
#include "opengl/gl_call.h"
#include "stb_image_write.h"
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>

const int BYTES_PER_PIXEL = 3;

AppStateSaveImageInfo AppStateSaveImageInfo_default(void) {
  return (AppStateSaveImageInfo){.path = "output.png", .to_save = false};
}

void AppState_save_image(AppStateSaveImageInfo *save_image_info, GLuint fbo,
                         OpenGLResolution resolution) {
  save_image_info->to_save = false;
  GL_CALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo));
  GL_CALL(glReadBuffer(GL_COLOR_ATTACHMENT0));
  GL_CALL(glPixelStorei(GL_PACK_ALIGNMENT, 1));

  void *pixels = malloc(resolution.width * resolution.height * BYTES_PER_PIXEL);

  GL_CALL(glReadPixels(0, 0, resolution.width, resolution.height, GL_RGB,
                       GL_UNSIGNED_BYTE, pixels));

  const int stride = resolution.width * 3;
  stbi_flip_vertically_on_write(true);
  if (stbi_write_png(save_image_info->path, resolution.width, resolution.height,
                     3, pixels, stride)) {
    printf("Sucessfully saved image to %s\n", save_image_info->path);
  }
  GL_CALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, 0));
  free(pixels);
}
