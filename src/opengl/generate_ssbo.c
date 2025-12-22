#include "opengl/generate_ssbo.h"
#include "opengl/gl_call.h"

void generate_ssbo(GLuint *ssbo, const void *data, int size, int index) {
  GL_CALL(glGenBuffers(1, ssbo));
  GL_CALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, *ssbo));
  GL_CALL(glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_DYNAMIC_DRAW));
  GL_CALL(glBindBufferRange(GL_SHADER_STORAGE_BUFFER, index, *ssbo, 0, size));
  GL_CALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
}
