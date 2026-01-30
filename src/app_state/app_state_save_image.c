#include "app_state/app_state_save_image.h"
#include "app_state.h"
#include "asserts.h"
#include "opengl/gl_call.h"
#include "opengl/resolution.h"
#include "renderer/parameters.h"
#include "stats.h"
#include "stb_image_write.h"
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>

const int BYTES_PER_PIXEL = 3;

AppStateSaveImageInfo AppStateSaveImageInfo_default(void) {
  return (AppStateSaveImageInfo){.path = "output.png", .to_save = false};
}

void add_metadata(const RendererParameters *renderer_parameters,
                  const char *image);

void AppState_save_image(AppState *app_state, GLuint fbo,
                         OpenGLResolution resolution) {
  app_state->save_image_info.to_save = false;
  GL_CALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo));
  GL_CALL(glReadBuffer(GL_COLOR_ATTACHMENT0));
  GL_CALL(glPixelStorei(GL_PACK_ALIGNMENT, 1));

  void *pixels = malloc(resolution.width * resolution.height * BYTES_PER_PIXEL);

  GL_CALL(glReadPixels(0, 0, resolution.width, resolution.height, GL_RGB,
                       GL_UNSIGNED_BYTE, pixels));

  StatsTimer_stop(&app_state->stats.rendering);

  const int stride = resolution.width * 3;
  stbi_flip_vertically_on_write(true);
  if (stbi_write_png(app_state->save_image_info.path, resolution.width,
                     resolution.height, 3, pixels, stride)) {
    printf("Sucessfully saved image to %s\n", app_state->save_image_info.path);
  }
  GL_CALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, 0));
  free(pixels);
  add_metadata(&app_state->rendering_params, app_state->save_image_info.path);
}

// NOTE: works only if exiftool is in PATH
void add_metadata(const RendererParameters *renderer_parameters,
                  const char *image_filename) {
  bool exiftool_available = false;
#ifdef __linux__
  exiftool_available = (system("exiftool -ver > /dev/null") == 0);
#elif defined(_WIN32)
  exiftool_available = (system("exiftool -ver >nul") == 0);
#endif
  if (exiftool_available) {
    char cmd[2048];
    char params[1024];

    ASSERTQ_CUSTOM(
        RendererParameters_str(renderer_parameters, params, sizeof(params)),
        "Buffer 'params' too small!");

    int to_write = snprintf(cmd, sizeof(cmd),
                            "exiftool -overwrite_original -Description='%s' %s",
                            params, image_filename);
    ASSERTQ_CUSTOM(to_write < (int)sizeof(cmd), "Buffer 'cmd' too small!");
    ASSERTQ_CUSTOM(system(cmd) == 0, "Command failed!");
  }
}
