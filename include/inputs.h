#ifndef INPUTS_H_
#define INPUTS_H_

#include "renderer.h"
#include <GLFW/glfw3.h>
#include <stdint.h>

typedef struct GLFWUserData {
  // 1 means true, -1 the opposite direction and 0 no
  int movingForward, movingLeft, movingUp;
  float lastMouseX, lastMouseY;
  float yaw, pitch;
  bool resetPosition, releaseCursor, paused;
} GLFWUserData;

void cursor_callback(GLFWwindow *window, double xPos, double yPos);
void cursor_enter_callback(GLFWwindow *window, int entered);
void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods);
bool update_inputs_uniforms(GLFWwindow *window, RUniforms *uniforms);

#endif // INPUTS_H_
