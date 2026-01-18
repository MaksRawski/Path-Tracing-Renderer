#include "renderer/shaders.h"
#include "opengl/gl_call.h"
#include "utils.h"
#include "utils/file_watcher.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// renders just pure white
#define DEFAULT_FRAGMENT_SHADER                                                \
  "#version 330 core\nvoid main(){gl_FragColor=vec4(1.0,1.0,1.0,1.0);}"
#define DEFAULT_VERTEX_SHADER                                                  \
  "#version 330 core\nlayout(location = 0) in vec3 aPos;\nvoid "               \
  "main(){gl_Position=vec4(aPos,1.0);}"

GLuint create_shader_program(const char *vertex_shader_filename,
                             const char *fragment_shader_filename);
GLuint create_shader_program_from_source(const char *vertex_shader_src,
                                         const char *fragment_shader_src);


RendererShaders RendererShaders_new(const char *vertex_shader_path,
                                    const char *fragment_shader_path) {
  RendererShaders self = {0};

  self.vertex_shader = FileWatcher_new(vertex_shader_path);
  self.fragment_shader = FileWatcher_new(fragment_shader_path);
  RendererShaders_force_update(&self);

  return self;
}

bool RendererShaders_update(RendererShaders *self) {
  if (FileWatcher_did_change(&self->vertex_shader) ||
      FileWatcher_did_change(&self->fragment_shader)) {
    printf("Shaders have changed, reloading...\n");
    RendererShaders_force_update(self);
    return true;
  }
  return false;
}

void RendererShaders_force_update(RendererShaders *self) {
  GLuint new_program =
      create_shader_program(self->vertex_shader.path, self->fragment_shader.path);

  if (new_program == (GLuint)-1) {
    fprintf(stderr,
            "Failed to create a shader program, loading the default...\n");
    new_program = create_shader_program_from_source(DEFAULT_VERTEX_SHADER,
                                                    DEFAULT_FRAGMENT_SHADER);
  }

  if (self->program != 0 && self->program != (GLuint)-1)
    GL_CALL(glDeleteProgram(self->program));

  self->program = new_program;
}

void RendererShaders_delete(RendererShaders *self) {
  FileWatcher_delete(&self->vertex_shader);
  FileWatcher_delete(&self->fragment_shader);
  GL_CALL(glDeleteProgram(self->program));
  self = NULL;
}


GLuint compile_shader(const char *shader_source, GLenum shader_type) {
  GLuint shader = glCreateShader(shader_type);
  GL_CALL(glShaderSource(shader, 1, &shader_source, NULL));
  GL_CALL(glCompileShader(shader));

  GLint success;
  GL_CALL(glGetShaderiv(shader, GL_COMPILE_STATUS, &success));
  if (!success) {
    GLchar infoLog[512];
    GL_CALL(glGetShaderInfoLog(shader, 512, NULL, infoLog));
    fprintf(stderr, "Error: Shader compilation failed\n%s", infoLog);
    return -1;
  }

  return shader;
}

GLuint create_shader_program_from_source(const char *vertex_shader_src,
                                         const char *fragment_shader_src) {

  GLuint vertex_shader = compile_shader(vertex_shader_src, GL_VERTEX_SHADER);
  GLuint fragment_shader =
      compile_shader(fragment_shader_src, GL_FRAGMENT_SHADER);

  if (vertex_shader == (GLuint)-1 || fragment_shader == (GLuint)-1)
    return -1;

  GLuint shaderProgram = glCreateProgram();
  GL_CALL(glAttachShader(shaderProgram, vertex_shader));
  GL_CALL(glAttachShader(shaderProgram, fragment_shader));
  GL_CALL(glLinkProgram(shaderProgram));

  GL_CALL(glDeleteShader(vertex_shader));
  GL_CALL(glDeleteShader(fragment_shader));

  GLint success;
  GL_CALL(glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success));
  if (!success) {
    GLchar infoLog[512];
    GL_CALL(glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog));
    fprintf(stderr, "Error: Shader program linking failed\n%s\n", infoLog);
    return -1;
  }

  return shaderProgram;
}

GLuint create_shader_program(const char *vertex_shader_filename,
                             const char *fragment_shader_filename) {

  char *vertex_shader_src = File_read(vertex_shader_filename);
  char *fragment_shader_src = File_read(fragment_shader_filename);

  GLuint shader_program =
      create_shader_program_from_source(vertex_shader_src, fragment_shader_src);

  free(vertex_shader_src);
  free(fragment_shader_src);

  return shader_program;
}
