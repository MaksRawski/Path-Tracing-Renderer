#include "opengl/gl_call.h"
#include "glad/gl.h"
#include <stdio.h>

void GL_CALL_impl(const char *file_name, int line_num) {
  GLenum gl_err = glGetError();
  if (gl_err != 0)
    fprintf(stderr, "%s:%d: OpenGL error 0x%x\n", file_name, line_num, gl_err);
}
