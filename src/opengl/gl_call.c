#include "opengl/gl_call.h"
#include "glad/gl.h"
#include <stdio.h>

void GL_CALL_impl(void) {
  GLenum gl_err = glGetError();
  if (gl_err != 0)
    fprintf(stderr, "%s:%d: OpenGL error 0x%x\n", __FILE__, __LINE__, gl_err);
}
