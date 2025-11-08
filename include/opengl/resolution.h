#ifndef RESOLUTION_H_
#define RESOLUTION_H_
#include <stdbool.h>

typedef struct {
  int width, height;
} OpenGLResolution;

OpenGLResolution OpenGLResolution_new(int width, int height);
bool OpenGLResolution_eq(OpenGLResolution a, OpenGLResolution b);

#endif // RESOLUTION_H_
