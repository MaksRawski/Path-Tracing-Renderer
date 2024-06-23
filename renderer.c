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

// Function to handle glfw errors
void glfw_error_callback(int error, const char *description) {
  fprintf(stderr, "Error (%d): %s\n", error, description);
}

GLFWwindow *setup_opengl(bool disable_vsync) {
  glfwSetErrorCallback(glfw_error_callback);

  if (!glfwInit())
    exit(EXIT_FAILURE);

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

  glGenBuffers(1, &rb->vao);
  glBindBuffer(GL_ARRAY_BUFFER, rb->vbo);
  glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), vertices, GL_STATIC_DRAW);

  glGenVertexArrays(1, &rb->vao);
  glBindVertexArray(rb->vao);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, rb->vbo);

  // index of the attribute? = 0, how many elements in row = 2, ..., sizeof each
  // "row", offset of each "row"
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), NULL);

  char *shader_src = read_shader_source(shader_filename);
  *shader_program = create_shader_program(shader_src);
  free(shader_src);
  *shader_watcher_fd = watch_shader_file(shader_filename);
}

void update_frame(GLuint shader_program, GLFWwindow *window, Uniforms *uniforms,
                  RendererBuffers *rb, BackBuffer *back_buffer,
                  ModelBuffer *mb) {
  /* glUseProgram(shader_program); */

  // render the quad to the back buffer
  glBindFramebuffer(GL_FRAMEBUFFER, back_buffer->fbo);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, back_buffer->fboTex);
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  // update uniforms
  glUseProgram(shader_program);
  glUniform1i(glGetUniformLocation(shader_program, "iFrame"), uniforms->iFrame);
  glUniform2f(glGetUniformLocation(shader_program, "iResolution"),
              uniforms->iResolution[0], uniforms->iResolution[1]);
  glUniform3f(glGetUniformLocation(shader_program, "camPos"),
              uniforms->camPos[0], uniforms->camPos[1], uniforms->camPos[2]);
  glUniform3f(glGetUniformLocation(shader_program, "camLookat"),
              uniforms->camLookat[0], uniforms->camLookat[1],
              uniforms->camLookat[2]);
  glUniform3f(glGetUniformLocation(shader_program, "camUp"), uniforms->camUp[0],
              uniforms->camUp[1], uniforms->camUp[2]);
  glUniform1f(glGetUniformLocation(shader_program, "camFov"), uniforms->camFov);

  // make sure the model is loaded
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_BUFFER, mb->tboTex);
  glUniform1i(glGetUniformLocation(shader_program, "trianglesBuffer"), 1);
  glUniform1i(glGetUniformLocation(shader_program, "numOfTriangles"), 1);

  // render the quad to the screen
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindVertexArray(rb->vao);
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  glfwSwapBuffers(window);
}

char *read_shader_source(const char *shader_file) {
  FILE *file = fopen(shader_file, "r");
  if (!file) {
    fprintf(stderr, "Error: Could not open shader file %s\n", shader_file);
    exit(EXIT_FAILURE);
  }

  fseek(file, 0, SEEK_END);
  long length = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *buffer = (char *)malloc(length + 1);
  if (!buffer) {
    fprintf(stderr, "Error: Could not allocate memory for shader source\n");
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
      char *fragment_shader_source = read_shader_source(shader_path);
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

void setup_back_buffer(BackBuffer *bb, unsigned int width,
                       unsigned int height) {
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
    printf("FPS: %.2f\n", fps);
    *frame_counter = 0;
    *last_frame_time = current_time;
  }
}

void free_gl_buffers(RendererBuffers *rb, BackBuffer *bb, ModelBuffer *mb) {
  glDeleteVertexArrays(1, &rb->vao);
  glDeleteBuffers(1, &rb->vbo);
  glDeleteFramebuffers(1, &bb->fbo);
  glDeleteTextures(1, &bb->fboTex);
  glDeleteBuffers(1, &mb->tbo);
  glDeleteTextures(1, &mb->tboTex);
}

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
  glBufferData(GL_ARRAY_BUFFER, sizeof(triangles), triangles,
               GL_STATIC_DRAW);

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