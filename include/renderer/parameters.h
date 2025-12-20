#ifndef RENDERER_PARAMETERS_H_
#define RENDERER_PARAMETERS_H_

#include "opengl/resolution.h"
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
/* RendererParameters RendererParameters_new(int max_bounce_count, */
/*                                           int samples_per_pixel, */
/*                                           float diverge_strength); */
/* bool RendererParameters_eq(RendererParameters a, RendererParameters b); */

#endif // RENDERER_PARAMETERS_H_
