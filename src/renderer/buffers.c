#include "renderer/buffers.h"
#include "renderer/buffers_scene.h"
#include <stddef.h>

RendererBuffers RendererBuffers_new(void) {
  RendererBuffers self = {.back = RendererBuffersBack_new(),
                          .internal = RendererBuffersInternal_new(),
                          .scene = {0}};
  return self;
}

void RendererBuffers_set_scene(RendererBuffers *self, const Scene *scene) {
  // delete the old scene buffer
  // IDEA: could instead do buffer data changes instead of recreating them
  if (scene->triangles_count != 0)
    RendererBuffersScene_delete(&self->scene);

  self->scene = RendererBuffersScene_new(scene);
}

void RendererBuffers_delete(RendererBuffers *self) {
  RendererBuffersBack_delete(&self->back);
  RendererBuffersInternal_delete(&self->internal);
  RendererBuffersScene_delete(&self->scene);

  self = NULL;
}
