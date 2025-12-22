#include "opengl/generate_ssbo.h"

void generate_ssbo(GLuint *ssbo, const void *data, int size, int index) {
  glGenBuffers(1, ssbo);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, *ssbo);
  glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_DYNAMIC_DRAW);
  glBindBufferRange(GL_SHADER_STORAGE_BUFFER, index, *ssbo, 0, size);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}
