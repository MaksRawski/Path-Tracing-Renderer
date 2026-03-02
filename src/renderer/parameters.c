#include "renderer/parameters.h"
#include "asserts.h"
#include "window/resolution.h"
#include <stdio.h>

RendererParameters RendererParameters_default(void) {
  return (RendererParameters){.max_bounce_count = 5,
                              .samples_per_pixel = 2,
                              .diverge_strength = 0.0005,
                              .frames_to_render = -1,
                              .rendering_resolution =
                                  WindowResolution_new(1280, 720)};
}

SmallString RendererParameters_str(const RendererParameters *self) {
  SmallString str = {0};
  int written = snprintf(str.str, sizeof(str.str),
                         "max_bounce_count: %d\nsamples_per_pixel: %d\n"
                         "diverge_strength: %.5f\nframes_to_render: %d\n",
                         self->max_bounce_count, self->samples_per_pixel,
                         self->diverge_strength, self->frames_to_render);
  ASSERTQ_CUSTOM(written < (int)sizeof(str.str),
                 "SmallString too small to store RendererParameters!");
  return str;
}
