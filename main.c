#include <glad/gl.h>
//
#include <GLFW/glfw3.h>
#include <stdio.h>

const char *fragment_shader = "#version 330\n"
                              "out vec4 frag_colour;"
                              "void main() {"
                              "  frag_colour = vec4(1, 0.0, 0, 1.0);"
                              "}";

// Function to handle errors
void error_callback(int error, const char *description) {
  fprintf(stderr, "Error (%d): %s\n", error, description);
}

int main(void) {
  glfwSetErrorCallback(error_callback);

  if (!glfwInit())
    return -1;

  GLFWwindow *window =
      glfwCreateWindow(600, 600, "OpenGL Full-Screen Quad", NULL, NULL);
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

  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fs, 1, &fragment_shader, NULL);
  glCompileShader(fs);

  GLuint shader_program = glCreateProgram();
  glAttachShader(shader_program, fs);
  glLinkProgram(shader_program);

  while (!glfwWindowShouldClose(window)) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    const float ratio = width / (float)height;

    glViewport(0, 0, width, height);

    glClear(GL_COLOR_BUFFER_BIT);
    // wipe the drawing surface clear
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shader_program);
    glBindVertexArray(vao);
    // draw points 0-3 from the currently bound VAO with current in-use shader
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    // put the stuff we've been drawing onto the display
    glfwSwapBuffers(window);
    // update other events like input handling
    glfwPollEvents();
  }

  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &vbo);
  glDeleteProgram(shader_program);
  glfwTerminate();
  return 0;
}
