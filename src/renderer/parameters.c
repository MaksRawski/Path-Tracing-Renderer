#include "renderer/parameters.h"

RendererParameters RendererParameters_default(void) {
  return (RendererParameters){
      .max_bounce_count = 5, .samples_per_pixel = 2, .diverge_strength = 0.001};
}

RendererParameters RendererParameters_new(int max_bounce_count,
                                          int samples_per_pixel,
                                          float diverge_strength) {
  return (RendererParameters){.max_bounce_count = max_bounce_count,
                              .samples_per_pixel = samples_per_pixel,
                              .diverge_strength = diverge_strength};
}

bool RendererParameters_eq(RendererParameters a, RendererParameters b) {
  return a.samples_per_pixel == b.samples_per_pixel &&
         a.diverge_strength == b.diverge_strength &&
         a.max_bounce_count == b.max_bounce_count;
}
