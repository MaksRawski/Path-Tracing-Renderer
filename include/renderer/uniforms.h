#ifndef RENDERER_UNIFORMS_H_
#define RENDERER_UNIFORMS_H_

#include "opengl/resolution.h"
#include "renderer/parameters.h"
#include <glad/gl.h>

// NOTE: this is purposefully not tightly coupled to any program
// instead update_in_program method expects the program to have these uniforms
// defined
typedef struct {
  unsigned int _iFrame;
  float _iResolution[2];
  RendererParameters _params;
} RendererUniforms;

RendererUniforms RendererUniforms_new(unsigned int width, unsigned int height);
void RendererUniforms_update(RendererUniforms *self, OpenGLResolution res,
                             RendererParameters params);
void RendererUniforms_update_in_program(const RendererUniforms *self,
                                        GLuint program);
void RendererUniforms_reset(RendererUniforms *self);
void RendererUniforms_delete(RendererUniforms *self);

#endif // RENDERER_UNIFORMS_H_
