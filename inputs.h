#ifndef INPUTS_H_
#define INPUTS_H_

#include "renderer.h"
#include <GLFW/glfw3.h>
#include <stdint.h>

typedef struct GLFWUserData {
  // 1 means true, -1 the opposite direction and 0 no
  int movingForward, movingLeft;
  float lastMouseX, lastMouseY;
  // yaw = 360 should point at the same point as yaw = 0
  // pitch = 0 means lookAt.y = cam.pos.y
  float yawDeg, pitchDeg;
} GLFWUserData;

typedef struct float3 {
  float x, y, z;
} float3;

void cursor_callback(GLFWwindow *window, double xPos, double yPos);
void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods);
bool update_uniforms(GLFWwindow *window, Uniforms *uniforms);

#endif // INPUTS_H_
