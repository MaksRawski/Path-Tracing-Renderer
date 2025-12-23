#ifndef RENDERER_PARAMETERS_H_
#define RENDERER_PARAMETERS_H_

#include "opengl/resolution.h"
#include <assert.h>
#include <stdbool.h>

typedef struct {
  int max_bounce_count;
  int samples_per_pixel;
  float diverge_strength;
  // -1 means infinite, NOTE: assuming progressive rendering
  int frames_to_render;
  OpenGLResolution rendering_resolution;
} RendererParameters;

RendererParameters RendererParameters_default(void);

#endif // RENDERER_PARAMETERS_H_
