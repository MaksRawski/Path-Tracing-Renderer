#ifndef RENDERER_BUFFERS_BACK_H_
#define RENDERER_BUFFERS_BACK_H_

#include <glad/gl.h> // GLuint

typedef struct {
  GLuint fbo, fboTex;
} RendererBuffersBack;

RendererBuffersBack RendererBuffersBack_new(unsigned int width, unsigned int height);
void RendererBuffersBack_delete(RendererBuffersBack *self);

#endif // RENDERER_BUFFERS_BACK_H_
