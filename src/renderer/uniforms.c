#include "renderer/uniforms.h"
#include "renderer/parameters.h"
#include <stddef.h>

RendererUniforms RendererUniforms_new(unsigned int width, unsigned int height) {
  return (RendererUniforms){._iFrame = 0,
                            ._iResolution = {width, height},
                            ._params = RendererParameters_default()};
}

void RendererUniforms_update(RendererUniforms *self, OpenGLResolution res,
                             RendererParameters params) {
  self->_iResolution[0] = res.width;
  self->_iResolution[1] = res.height;
  self->_params = params;
}

void RendererUniforms_reset(RendererUniforms *self) { self->_iFrame = 0; }

void RendererUniforms_update_in_program(const RendererUniforms *self,
                                        GLuint program) {
  glUniform1i(glGetUniformLocation(program, "iFrame"), self->_iFrame);
  glUniform2f(glGetUniformLocation(program, "iResolution"),
              self->_iResolution[0], self->_iResolution[1]);
  glUniform1i(glGetUniformLocation(program, "MAX_BOUNCE_COUNT"),
              self->_params.max_bounce_count);
  glUniform1i(glGetUniformLocation(program, "SAMPLES_PER_PIXEL"),
              self->_params.samples_per_pixel);
  glUniform1f(glGetUniformLocation(program, "DIVERGE_STRENGTH"),
              self->_params.diverge_strength);
}

void RendererUniforms_delete(RendererUniforms *self) {
  (void)(self);
  self = NULL;
}
