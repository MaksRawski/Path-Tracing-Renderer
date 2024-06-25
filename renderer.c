#include <glad/gl.h>
//
#include <GLFW/glfw3.h>
//
#include "renderer.h"
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
// for hot reloading
#include <fcntl.h>
#include <sys/inotify.h>

#define WINDOW_TITLE "LAK - Projekt zaliczeniowy"

int debug_fn_counter = 0;

void debug() {
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
    fprintf(stderr, "%d. OpenGL Error: %x\n", ++debug_fn_counter, error);
  else
    printf("%d. all good\n", ++debug_fn_counter);
}

// Function to handle glfw errors
void glfw_error_callback(int error, const char *description) {
  fprintf(stderr, "Error (%d): %s\n", error, description);
}

GLFWwindow *setup_opengl(bool disable_vsync) {
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit())
    exit(EXIT_FAILURE);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(600, 600, WINDOW_TITLE, NULL, NULL);
  if (!window) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(window);
  int version = gladLoadGL(glfwGetProcAddress);
  if (version == 0) {
    printf("Failed to initialize OpenGL context\n");
    exit(EXIT_FAILURE);
  }

  // disable vsync to see the full speed
  if (disable_vsync)
    glfwSwapInterval(0);

  // Successfully loaded OpenGL
  printf("Loaded OpenGL %d.%d\n", GLAD_VERSION_MAJOR(version),
         GLAD_VERSION_MINOR(version));

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  return window;
}

void setup_renderer(const char *shader_filename, GLuint *shader_program,
                    int *shader_watcher_fd, RendererBuffers *rb) {
  // Define vertices for a full-screen quad
  float vertices[] = {
      -1.0f, 1.0f,  // Top left
      1.0f,  1.0f,  // Top right
      1.0f,  -1.0f, // Bottom right
      -1.0f, -1.0f  // Bottom left
  };

  // generate and bind vertex array object
  glGenVertexArrays(1, &rb->vao);
  glBindVertexArray(rb->vao);

  // generate and bind vertex buffer object
  glGenBuffers(1, &rb->vbo);
  glBindBuffer(GL_ARRAY_BUFFER, rb->vbo);

  // upload vertex data to VBO
  glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), vertices, GL_STATIC_DRAW);

  // index of the attribute = 0, how many elements in row = 2, ..., sizeof each
  // "row", offset of each "row"
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), NULL);

  char *shader_src = read_file(shader_filename);
  *shader_program = create_shader_program(shader_src);
  free(shader_src);
  *shader_watcher_fd = watch_shader_file(shader_filename);

  // unbind the vao
  glBindVertexArray(0);

  // unbind the vbo
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void update_frame(GLuint shader_program, GLFWwindow *window, Uniforms *uniforms,
                  RendererBuffers *rb, BackBuffer *back_buffer,
                  ModelsBuffer *mb) {
  // setup the program and bind the vao associated with the quad
  // and the vbo holding the vertices of the quad
  glUseProgram(shader_program);
  glBindVertexArray(rb->vao);
  glBindBuffer(GL_ARRAY_BUFFER, rb->vbo); // TODO: this is probably unnecessary

  // render the quad to the back buffer
  glBindFramebuffer(GL_FRAMEBUFFER, back_buffer->fbo);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, back_buffer->fboTex);
  /* glUniform1i(glGetUniformLocation(shader_program, "BackBufferTexture"), 0); */
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  // update uniforms
  glUseProgram(shader_program);
  /* glUniform1i(glGetUniformLocation(shader_program, "iFrame"), uniforms->iFrame); */
  /* glUniform2f(glGetUniformLocation(shader_program, "iResolution"), */
  /*             uniforms->iResolution[0], uniforms->iResolution[1]); */

  // make sure the models' "textures" are loaded
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_BUFFER, mb->tbo_tex_triangles);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_BUFFER, mb->tbo_tex_meshes);
  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_BUFFER, mb->tbo_tex_materials);

  /* glUniform1i(glGetUniformLocation(shader_program, "trianglesBuffer"), 1); */
  /* glUniform1i(glGetUniformLocation(shader_program, "meshesInfoBuffer"), 2); */
  /* glUniform1i(glGetUniformLocation(shader_program, "materialsBuffer"), 3); */
  /* glUniform1i(glGetUniformLocation(shader_program, "numOfMeshes"), */
  /*             mb->num_of_meshes); */

  // render the quad to the screen
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindVertexArray(rb->vao);
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  glfwSwapBuffers(window);
}

char *read_file(const char *filename) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    fprintf(stderr, "Error: Could not open file %s\n", filename);
    exit(EXIT_FAILURE);
  }

  fseek(file, 0, SEEK_END);
  long length = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *buffer = (char *)malloc(length + 1);
  if (!buffer) {
    fprintf(stderr, "Error: Could not allocate memory for file %s\n", filename);
    exit(EXIT_FAILURE);
  }

  fread(buffer, 1, length, file);
  buffer[length] = '\0';

  fclose(file);
  return buffer;
}

GLuint compile_shader(const char *shaderSource, GLenum shaderType) {
  GLuint shader = glCreateShader(shaderType);
  glShaderSource(shader, 1, &shaderSource, NULL);
  glCompileShader(shader);

  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    GLchar infoLog[512];
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    fprintf(stderr, "Error: Shader compilation failed\n%s\n", infoLog);
    return -1;
  }

  return shader;
}

GLuint create_shader_program(const char *fragment_shader_source) {
  GLuint fragmentShader =
      compile_shader(fragment_shader_source, GL_FRAGMENT_SHADER);

  if (fragmentShader == (GLuint)-1)
    return -1;

  GLuint shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  glDeleteShader(fragmentShader);

  GLint success;
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    GLchar infoLog[512];
    glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
    fprintf(stderr, "Error: Shader program linking failed\n%s\n", infoLog);
    return -1;
  }

  return shaderProgram;
}

bool reload_shader(int watcher_fd, GLuint *program, const char *shader_path) {
  char buffer[BUF_LEN];
  int length = read(watcher_fd, buffer, BUF_LEN);
  if (length < 0) {
    if (errno != EAGAIN)
      perror("read");
    return false;
  }

  int i = 0;
  bool did_reload = false;
  while (i < length) {
    struct inotify_event *event = (struct inotify_event *)&buffer[i];
    if (event->mask & IN_MODIFY) {
      printf("%s modified, recompiling...\n", shader_path);
      char *fragment_shader_source = read_file(shader_path);
      GLuint new_program = create_shader_program(fragment_shader_source);
      free(fragment_shader_source);

      if (new_program == (GLuint)-1)
        new_program = create_shader_program(DEFAULT_SHADER_PROGRAM);

      glDeleteProgram(*program);
      did_reload = true;
      *program = new_program;
    }
    i += EVENT_SIZE + event->len;
  }
  return did_reload;
}

int watch_shader_file(const char *shader_path) {
  int fd = inotify_init();
  if (fd < 0) {
    perror("inotify_init");
    exit(EXIT_FAILURE);
  }

  int wd = inotify_add_watch(fd, shader_path, IN_MODIFY);
  if (wd == -1) {
    fprintf(stderr, "Cannot watch '%s'\n", shader_path);
    exit(EXIT_FAILURE);
  }

  int flags = fcntl(fd, F_GETFL, 0);
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);

  return fd;
}

void setup_back_buffer(GLuint shader_program, BackBuffer *bb,
                       unsigned int width, unsigned int height) {
  glGenFramebuffers(1, &bb->fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, bb->fbo);

  // create the texture for the framebuffer
  glGenTextures(1, &bb->fboTex);
  glBindTexture(GL_TEXTURE_2D, bb->fboTex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
               GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);

  // attach the texture to our framebuffer
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         bb->fboTex, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // set the program uniform for the texture sampler
  glUseProgram(shader_program);
  glUniform1i(glGetUniformLocation(shader_program, "BackBufferTexture"), 0);
}

void display_fps(GLFWwindow *window, unsigned int *frame_counter,
                 double *last_frame_time) {
  double current_time = glfwGetTime();
  double delta_time = current_time - *last_frame_time;
  char window_title[40];

  if (delta_time >= 2.0) {
    double fps = *frame_counter / delta_time;
    sprintf(window_title, "%s [%.2f FPS]", WINDOW_TITLE, fps);
    glfwSetWindowTitle(window, window_title);
#ifdef LOG_FPS
    printf("FPS: %.2f\n", fps);
#endif
    *frame_counter = 0;
    *last_frame_time = current_time;
  }
}

void free_gl_buffers(RendererBuffers *rb, BackBuffer *bb, ModelsBuffer *mb) {
  glDeleteVertexArrays(1, &rb->vao);
  glDeleteBuffers(1, &rb->vbo);
  glDeleteFramebuffers(1, &bb->fbo);
  glDeleteTextures(1, &bb->fboTex);
  glDeleteBuffers(1, &mb->tbo_triangles);
  glDeleteTextures(1, &mb->tbo_tex_triangles);
  glDeleteBuffers(1, &mb->tbo_meshes);
  glDeleteTextures(1, &mb->tbo_tex_meshes);
  glDeleteBuffers(1, &mb->tbo_meshes);
  glDeleteTextures(1, &mb->tbo_tex_meshes);
  free(mb->triangles);
  free(mb->meshesInfo);
  free(mb->materials);
}
