#ifndef PARAMETERS_BUFFER_H_
#define PARAMETERS_BUFFER_H_

#include "glad/gl.h"
#include "renderer/parameters.h"

typedef struct {
  GLuint params_ssbo;
} RendererParametersBuffer;

RendererParametersBuffer RendererParametersBuffer_default(void);
void RendererParametersBuffer_set(RendererParametersBuffer *self,
                                  const RendererParameters *params);
void RendererParametersBuffer_delete(RendererParametersBuffer *self);

#endif // PARAMETERS_BUFFER_H_
