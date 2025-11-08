#include "renderer/uniforms.h"
#include <stddef.h>

RendererUniforms RendererUniforms_new(unsigned int width, unsigned int height) {
  return (RendererUniforms){._iFrame = 0, ._iResolution = {width, height}};
}

void RendererUniforms_update(RendererUniforms *self, OpenGLResolution res) {
  ++self->_iFrame;
  self->_iResolution[0] = res.width;
  self->_iResolution[1] = res.height;
}

void RendererUniforms_reset(RendererUniforms *self) {
  self->_iFrame = 0;
}

void RendererUniforms_update_in_program(const RendererUniforms *self,
                                        GLuint program) {
  glUniform1i(glGetUniformLocation(program, "iFrame"), self->_iFrame);
  glUniform2f(glGetUniformLocation(program, "iResolution"),
              self->_iResolution[0], self->_iResolution[1]);
}

void RendererUniforms_delete(RendererUniforms *self) {
  (void)(self);
  self = NULL;
}
