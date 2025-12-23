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
