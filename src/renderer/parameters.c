#include "renderer/parameters.h"
#include "opengl/resolution.h"

RendererParameters RendererParameters_default(void) {
  return (RendererParameters){.max_bounce_count = 5,
                              .samples_per_pixel = 2,
                              .diverge_strength = 0.001,
                              .frames_to_render = -1,
                              .rendering_resolution =
                                  OpenGLResolution_new(1280, 720)};
}

RendererParameters RendererParameters_new(int max_bounce_count,
                                          int samples_per_pixel,
                                          float diverge_strength,
                                          int frames_to_render) {
  return (RendererParameters){.max_bounce_count = max_bounce_count,
                              .samples_per_pixel = samples_per_pixel,
                              .diverge_strength = diverge_strength,
                              .frames_to_render = frames_to_render};
}

/* bool RendererParameters_eq(RendererParameters a, RendererParameters b) { */
/*   return a.samples_per_pixel == b.samples_per_pixel && */
/*          a.diverge_strength == b.diverge_strength && */
/*          a.max_bounce_count == b.max_bounce_count && */
/*          a.frames_to_render == b.frames_to_render; */
/* } */
