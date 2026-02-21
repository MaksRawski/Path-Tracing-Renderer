#ifndef RENDERER_PARAMETERS_H_
#define RENDERER_PARAMETERS_H_

#include "window/resolution.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  float env_color[3];
  int32_t max_bounce_count;
  int32_t samples_per_pixel;
  float diverge_strength;
  // -1 means infinite, NOTE: assuming progressive rendering
  int32_t frames_to_render;
  WindowResolution rendering_resolution;
} RendererParameters;

RendererParameters RendererParameters_default(void);
bool RendererParameters_str(const RendererParameters *self, char *out,
                            unsigned long buf_size);

#endif // RENDERER_PARAMETERS_H_
