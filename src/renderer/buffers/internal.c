#include "renderer/buffers/internal.h"
#include "opengl/gl_call.h"
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
  GL_CALL(glGenVertexArrays(1, &self.vao));
  // generate vertex buffer object
  GL_CALL(glGenBuffers(1, &self.vbo));

  // bind both buffers (order is important!)
  GL_CALL(glBindVertexArray(self.vao));
  GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, self.vbo));

  // upload vertex data to VBO
  GL_CALL(glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), vertices, GL_STATIC_DRAW));

  // index of the attribute = 0 (will be used in the vertex shader)
  // how many values per vertex = 2,
  // type of each element = GL_FLOAT,
  // do we have coordinates as integers?,
  // stride, space between every vertex
  GL_CALL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), NULL));
  GL_CALL(glEnableVertexAttribArray(0));

  // unbind the vbo
  GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));

  // unbind the vao
  GL_CALL(glBindVertexArray(0));

  return self;
}

void RendererBuffersInternal_delete(RendererBuffersInternal *self) {
  GL_CALL(glDeleteVertexArrays(1, &self->vao));
  GL_CALL(glDeleteBuffers(1, &self->vbo));

  self = NULL;
}
