#include <glad/gl.h>

#include "obj_parser.h"
#include "renderer.h"

void load_obj_model(const char *filename, GLuint shader_program,
                    ModelBuffer *mb) {
  Triangle triangles[1] = {{
      {20.0f, -1.0f, -5.0f}, // a (left)
      {17.5f, 5.0f, 10.0f},  // b (top)
      {10.0f, 1.0f, 10.0f},  // c (right)
      {0.0f, 1.0f, 0.0f},    // na
      {0.0f, 1.0f, 0.0f},    // nb
      {0.0f, 1.0f, 0.0f}     // nc
  }};
  // to store vertices
  glGenBuffers(1, &mb->tbo);
  glBindBuffer(GL_ARRAY_BUFFER, mb->tbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(triangles), triangles, GL_STATIC_DRAW);

  // texture buffer object, will bind the vbo to it
  // so that the shader can sample the texture and actually
  // get the data from the vbo
  glGenTextures(1, &mb->tboTex);
  glBindTexture(GL_TEXTURE_BUFFER, mb->tboTex);
  // we choose GL_RGB32F to have each coordinate store 3 floats AKA vec3
  // and then use vbo as data for that texture,
  glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, mb->tbo);

  // bind the texture as a uniform in the shader
  glUseProgram(shader_program);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_BUFFER, mb->tboTex);
  // set to texture at index 1 as index 0 we will be storing back buffer
  glUniform1i(glGetUniformLocation(shader_program, "trianglesBuffer"), 1);
  glUniform1i(glGetUniformLocation(shader_program, "numOfTriangles"), 2);

  // unbind the texture buffer
  glBindBuffer(GL_TEXTURE_BUFFER, 0);
}
