#ifndef INPUTS_H_
#define INPUTS_H_


#include "renderer.h"
#include <stdint.h>
#include <GLFW/glfw3.h>

typedef struct GLFWUserData {
  // 1 means true, -1 the opposite direction and 0 no
  int movingForward, movingLeft;
} GLFWUserData;

typedef struct float3 {
  float x, y, z;
} float3;

bool update_uniforms(GLFWwindow *window, Uniforms *uniforms);


#endif // INPUTS_H_
