#ifndef RENDERER_PARAMETERS_H_
#define RENDERER_PARAMETERS_H_

#include <stdbool.h>

typedef struct {
  int max_bounce_count;
  int samples_per_pixel;
  float diverge_strength;
} RendererParameters;

RendererParameters RendererParameters_default(void);
RendererParameters RendererParameters_new(int max_bounce_count,
                                          int samples_per_pixel,
                                          float diverge_strength);
bool RendererParameters_eq(RendererParameters a, RendererParameters b);

#endif // RENDERER_PARAMETERS_H_
