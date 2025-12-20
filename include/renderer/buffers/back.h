#ifndef RENDERER_BUFFERS_BACK_H_
#define RENDERER_BUFFERS_BACK_H_

#include "opengl/resolution.h"
#include <glad/gl.h> // GLuint

typedef struct {
  GLuint fbo, fboTex;
} RendererBuffersBack;

RendererBuffersBack RendererBuffersBack_new(void);
void RendererBuffersBack_resize(RendererBuffersBack *self,
                                OpenGLResolution res);

void RendererBuffersBack_delete(RendererBuffersBack *self);

#endif // RENDERER_BUFFERS_BACK_H_
