#ifndef RESOLUTION_H_
#define RESOLUTION_H_
#include <stdbool.h>

typedef struct {
  unsigned int width, height;
} OpenGLResolution;

OpenGLResolution OpenGLResolution_new(unsigned int width, unsigned int height);
bool OpenGLResolution_eq(OpenGLResolution a, OpenGLResolution b);

#endif // RESOLUTION_H_
