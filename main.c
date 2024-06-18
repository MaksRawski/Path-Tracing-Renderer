#include <glad/gl.h>
//
#include <GLFW/glfw3.h>
// for hot reloading
#include <fcntl.h>
#include <sys/inotify.h>
//
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

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

GLuint create_shader_program(const char *fragment_shader_path) {
  char *fragment_shader_source = read_shader_source(fragment_shader_path);

  GLuint fragmentShader =
      compile_shader(fragment_shader_source, GL_FRAGMENT_SHADER);

  if (fragmentShader < 0) return -1;

  GLuint shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  GLint success;
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    GLchar infoLog[512];
    glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
    fprintf(stderr, "Error: Shader program linking failed\n%s\n", infoLog);
    /* exit(EXIT_FAILURE); */
  }

  glDeleteShader(fragmentShader);
  free(fragment_shader_source);

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
      GLuint new_program = create_shader_program(shader_path);
      if (new_program) {
        glDeleteProgram(*program);
        *program = new_program;
        did_reload = true;
      }
    }
    i += EVENT_SIZE + event->len;
  }
  return did_reload;
}

// Function to handle errors
void error_callback(int error, const char *description) {
  fprintf(stderr, "Error (%d): %s\n", error, description);
}

int main(void) {

  glfwSetErrorCallback(error_callback);

  if (!glfwInit())
    return -1;

  GLFWwindow *window =
      glfwCreateWindow(600, 600, "LAK - Projekt zaliczeniowy", NULL, NULL);
  if (!window) {
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);
  int version = gladLoadGL(glfwGetProcAddress);
  if (version == 0) {
    printf("Failed to initialize OpenGL context\n");
    return -1;
  }

  // Successfully loaded OpenGL
  printf("Loaded OpenGL %d.%d\n", GLAD_VERSION_MAJOR(version),
         GLAD_VERSION_MINOR(version));

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  // Define vertices for a full-screen quad
  float vertices[] = {
      -1.0f, 1.0f,  // Top left
      1.0f,  1.0f,  // Top right
      1.0f,  -1.0f, // Bottom right
      -1.0f, -1.0f  // Bottom left
  };

  GLuint vbo = 0; // vertex buffer object
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), vertices, GL_STATIC_DRAW);

  GLuint vao = 0; // vertex array object, one per mesh
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  // index of the attribute? = 0, how many elements in row = 2, ..., sizeof each
  // "row", offset of each "row"
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), NULL);

  GLuint shader_program = create_shader_program("renderer.glsl");
  int shader_watcher_fd = watch_shader_file("renderer.glsl");

  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  glViewport(0, 0, width, height);

  // BACK BUFFER
  GLuint fbo; // framebuffer object
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  // create the texture for the framebuffer
  GLuint fbo_texture;
  glGenTextures(1, &fbo_texture);
  glBindTexture(GL_TEXTURE_2D, fbo_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
               GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0); // ???

  // attach the texture to our framebuffer
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         fbo_texture, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // FPS COUNTING SETUP
  unsigned int iFrame = 0;
  unsigned int fps_counter = 0;
  double fps;
  double last_frame_time = glfwGetTime();

  while (!glfwWindowShouldClose(window)) {
    bool did_reload =
        reload_shader(shader_watcher_fd, &shader_program, "renderer.glsl");

    // recalculate window dimensions
    int new_width, new_height;
    glfwGetFramebufferSize(window, &new_width, &new_height);
    if (new_width != width || new_height != height) {
      width = new_width;
      height = new_height;

      // Resize the backbuffer texture
      glBindTexture(GL_TEXTURE_2D, fbo_texture);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                   GL_UNSIGNED_BYTE, NULL);
      glBindTexture(GL_TEXTURE_2D, 0);

      glViewport(0, 0, width, height);
      iFrame = 0;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // CALCUALTE FPS
    double current_time = glfwGetTime();
    double delta_time = current_time - last_frame_time;

    if (delta_time >= 2.0) {
      fps = fps_counter / delta_time;
      printf("FPS: %.2f\n", fps);
      fps_counter = 0;
      last_frame_time = current_time;
    }

    if (did_reload) {
      iFrame = 0;
      fps_counter = 0;
      last_frame_time = current_time;
    }


    // DRAWING THE FRAME
    // render the quad to the back buffer
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fbo_texture);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // update uniforms
    glUseProgram(shader_program);
    glUniform1i(glGetUniformLocation(shader_program, "iFrame"), iFrame);
    glUniform2f(glGetUniformLocation(shader_program, "iResolution"), width,
                height);
    glUniform1i(glGetUniformLocation(shader_program, "iChannel0"), 0);

    // render the quad to the screen
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glfwSwapBuffers(window);
    ++fps_counter;

    glfwPollEvents();
    ++iFrame;
  }

  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &vbo);
  glDeleteProgram(shader_program);
  glDeleteFramebuffers(1, &fbo);
  glDeleteTextures(1, &fbo_texture);
  glfwTerminate();
  close(shader_watcher_fd);

  return 0;
}
