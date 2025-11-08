#include "renderer/buffers/internal.h"
#include <stddef.h>

RendererBuffersInternal RendererBuffersInternal_new(void) {
  RendererBuffersInternal self = {0};

  // define vertices for a full-screen quad
  float vertices[] = {
      -1.0f, 1.0f,  // top left
      1.0f,  1.0f,  // top right
      1.0f,  -1.0f, // bottom right
      -1.0f, -1.0f  // bottom left
  };

  // generate vertex array object
  glGenVertexArrays(1, &self.vao);
  // generate vertex buffer object
  glGenBuffers(1, &self.vbo);

  // bind both buffers (order is important!)
  glBindVertexArray(self.vao);
  glBindBuffer(GL_ARRAY_BUFFER, self.vbo);

  // upload vertex data to VBO
  glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), vertices, GL_STATIC_DRAW);

  // index of the attribute = 0 (will be used in the vertex shader)
  // how many values per vertex = 2,
  // type of each element = GL_FLOAT,
  // do we have coordinates as integers?,
  // stride, space between every vertex
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), NULL);
  glEnableVertexAttribArray(0);

  // unbind the vbo
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // unbind the vao
  glBindVertexArray(0);

  return self;
}

void RendererBuffersInternal_delete(RendererBuffersInternal *self) {
  glDeleteVertexArrays(1, &self->vao);
  glDeleteBuffers(1, &self->vbo);

  self = NULL;
}
