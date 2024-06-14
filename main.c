#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

const int WIDTH = 800;
const int HEIGHT = 600;

void glfw_error_callback(int error, const char *description) {
  fprintf(stderr, "GLFW Error: %s\n", description);
}

int main(int argc, char *argv[]) {
  if (!glfwInit()) {
    fprintf(stderr, "Nie udało się zainicjować GLFW!\n");
    return -1;
  }
  // Upewniamy się, że wersja OpenGL to co najmniej 3.x.
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(
      WIDTH, HEIGHT, "RT Renderer - Projekt zaliczeniowy na LAK", NULL, NULL);

  if (!window) {
    fprintf(stderr, "Nie udało się utworzyć okienka GLFW!\n");
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);

  while (!glfwWindowShouldClose(window)) {
    //
  }
}
