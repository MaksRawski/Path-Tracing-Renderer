#include <glad/gl.h>
//
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

char *readShaderSource(const char *shaderFile) {
  FILE *file = fopen(shaderFile, "r");
  if (!file) {
    fprintf(stderr, "Error: Could not open shader file %s\n", shaderFile);
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

GLuint compileShader(const char *shaderSource, GLenum shaderType) {
  GLuint shader = glCreateShader(shaderType);
  glShaderSource(shader, 1, &shaderSource, NULL);
  glCompileShader(shader);

  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    GLchar infoLog[512];
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    fprintf(stderr, "Error: Shader compilation failed\n%s\n", infoLog);
    exit(EXIT_FAILURE);
  }

  return shader;
}

GLuint createShaderProgram(const char *fragmentShaderPath) {
  char *fragmentShaderSource = readShaderSource(fragmentShaderPath);

  GLuint fragmentShader =
      compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);

  GLuint shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  GLint success;
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    GLchar infoLog[512];
    glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
    fprintf(stderr, "Error: Shader program linking failed\n%s\n", infoLog);
    exit(EXIT_FAILURE);
  }

  glDeleteShader(fragmentShader);
  free(fragmentShaderSource);

  return shaderProgram;
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

  GLuint shader_program = createShaderProgram("renderer.glsl");

  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  glViewport(0, 0, width, height);

  // BACK BUFFER
  GLuint fbo; // framebuffer object
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  // create the texture for the framebuffer
  GLuint fboTexture;
  glGenTextures(1, &fboTexture);
  glBindTexture(GL_TEXTURE_2D, fboTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
               GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0); // ???

  // attach the texture to our framebuffer
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         fboTexture, 0);

  /* // Check if framebuffer is complete (whatever that means) */
  /* if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
   */
  /*   fprintf(stderr, "Error: Framebuffer is not complete!\n"); */
  /*   exit(EXIT_FAILURE); */
  /* } */
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // FPS COUNTING SETUP
  unsigned int iFrame = 0;
  unsigned int fps_counter = 0;
  double fps;
  double last_frame_time = glfwGetTime();

  while (!glfwWindowShouldClose(window)) {
    // recalculate window dimensions
    int new_width, new_height;
    glfwGetFramebufferSize(window, &new_width, &new_height);
    if (new_width != width || new_height != height) {
      width = new_width;
      height = new_height;

      // Resize the backbuffer texture
      glBindTexture(GL_TEXTURE_2D, fboTexture);
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

    // DRAWING THE FRAME

    // render the quad to the back buffer
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fboTexture);
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
  glDeleteTextures(1, &fboTexture);
  glfwTerminate();

  return 0;
}
