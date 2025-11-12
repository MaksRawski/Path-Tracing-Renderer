#include "renderer.h"

#include "inputs.h"
#include <GLFW/glfw3.h>

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
  GLFWUserData *user_data = glfwGetWindowUserPointer(window);

  // NOTE: I have caps lock and escape swapped but GLFW takes
  // hardware keys so leaving this hack in so I don't go crazy
  if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_CAPS_LOCK) {
    user_data->paused = true;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  }

  Renderer_handle_key_callback(user_data, key, scancode, action, mods);
}

void cursor_callback(GLFWwindow *window, double xPos, double yPos) {
  GLFWUserData *userPtr = glfwGetWindowUserPointer(window);
  if (userPtr->paused)
    return;
  Renderer_handle_cursor_callback(userPtr, xPos, yPos);
}

void cursor_enter_callback(GLFWwindow *window, int entered) {
  GLFWUserData *ptr = (GLFWUserData *)glfwGetWindowUserPointer(window);
  if (entered) {
    // hide cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    ptr->paused = false;
  }
}
