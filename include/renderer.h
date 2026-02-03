#ifndef RENDERER_H_
#define RENDERER_H_

#include "renderer/buffers.h"
#include "renderer/parameters.h"
#include "renderer/shaders.h"

typedef struct {
  RendererShaders _shaders;
  RendererBuffers _buffers;
  OpenGLResolution _res;
} Renderer;

Renderer Renderer_new(void);

void Renderer_load_scene(Renderer *self, const Scene *scene);
void Renderer_set_camera(Renderer *self, Camera cam);
void Renderer_set_params(Renderer *self, RendererParameters params);

void Renderer_render_frame(const Renderer *self, unsigned int frame_number);
GLuint Renderer_get_fbo(const Renderer *self);

void Renderer_clear_backbuffer(Renderer *self);

void Renderer_delete(Renderer *self);

#endif // RENDERER_H_
