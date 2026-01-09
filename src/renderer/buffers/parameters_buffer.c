#include "renderer/buffers/parameters_buffer.h"
#include "opengl/generate_ssbo.h"
#include "opengl/gl_call.h"
#include "renderer/parameters.h"
#include <stddef.h>

#include "glad/gl.h"

RendererParametersBuffer RendererParametersBuffer_default(void) {
  RendererParametersBuffer self = {0};
  RendererParameters params = RendererParameters_default();
  generate_ssbo(&self.params_ssbo, &params, sizeof(params), 10);
  return self;
}

void RendererParametersBuffer_set(RendererParametersBuffer *self,
                                  const RendererParameters *params) {
  GL_CALL(glNamedBufferSubData(self->params_ssbo, 0, sizeof(RendererParameters),
                               params));
}

void RendererParametersBuffer_delete(RendererParametersBuffer *self) {
  GL_CALL(glDeleteBuffers(1, &self->params_ssbo));

  self = NULL;
}
