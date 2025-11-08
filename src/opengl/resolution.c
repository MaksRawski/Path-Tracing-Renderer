#include "opengl/resolution.h"

OpenGLResolution OpenGLResolution_new(int width, int height) {
  return (OpenGLResolution){width, height};
}

bool OpenGLResolution_eq(OpenGLResolution a, OpenGLResolution b) {
  return (a.width == b.width && a.height == b.height);
}
