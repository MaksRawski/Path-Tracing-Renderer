#ifndef RENDERER_BUFFERS_INTERNAL_H_
#define RENDERER_BUFFERS_INTERNAL_H_

#include <glad/gl.h> // GLuint

typedef struct {
  GLuint vao;
  GLuint vbo;
} RendererBuffersInternal;

RendererBuffersInternal RendererBuffersInternal_new(void);
void RendererBuffersInternal_delete(RendererBuffersInternal *self);

#endif // RENDERER_BUFFERS_INTERNAL_H_
