#ifndef BUFFERS_H_
#define BUFFERS_H_

#include "scene.h"

#include "renderer/buffers_scene.h" 
#include "renderer/buffers/back.h"
#include "renderer/buffers/internal.h"

typedef struct {
  RendererBuffersScene scene;
  RendererBuffersBack back;
  RendererBuffersInternal internal;
} RendererBuffers;

RendererBuffers RendererBuffers_new(void);
void RendererBuffers_set_scene(RendererBuffers *self, const Scene *scene);
void RendererBuffers_delete(RendererBuffers *self);

#endif // BUFFERS_H_
