#include "renderer/parameters.h"
#include "opengl/resolution.h"
#include <stdio.h>

RendererParameters RendererParameters_default(void) {
  return (RendererParameters){.max_bounce_count = 5,
                              .samples_per_pixel = 2,
                              .diverge_strength = 0.001,
                              .frames_to_render = -1,
                              .rendering_resolution =
                                  OpenGLResolution_new(1280, 720)};
}

// NOTE: the buffer should at the very least allocate 128 bytes
// writes at max `buf_size` bytes into `out` and return whether the output did
// fit.
bool RendererParameters_str(const RendererParameters *self, char *out,
                            unsigned long buf_size) {
  // NOTE: str_len is the length of the string that should have been written.
  int str_len = snprintf(out, buf_size,
                         "max_bounce_count: %d\nsamples_per_pixel: %d\n"
                         "diverge_strength: %.5f\nframes_to_render: %d\n",
                         self->max_bounce_count, self->samples_per_pixel,
                         self->diverge_strength, self->frames_to_render);
  return str_len < (int)buf_size;
}
