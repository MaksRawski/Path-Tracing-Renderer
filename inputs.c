#include "inputs.h"
#include "renderer.h"
#include <GLFW/glfw3.h>
#include <math.h>
#include <stdio.h>

const float STEP_SIZE_PER_FRAME = 0.05;

void moveForward(Uniforms *uniforms, float dir) {
  float diffX = uniforms->camPos[0] - uniforms->camLookat[0];
  float diffY = uniforms->camPos[1] - uniforms->camLookat[1];
  float diffZ = uniforms->camPos[2] - uniforms->camLookat[2];

  if (diffX > 0)
    uniforms->camPos[0] -= STEP_SIZE_PER_FRAME * dir;
  else if (diffX < 0)
    uniforms->camPos[0] += STEP_SIZE_PER_FRAME * dir;

  if (diffY > 0)
    uniforms->camPos[1] -= STEP_SIZE_PER_FRAME * dir;
  else if (diffY < 0)
    uniforms->camPos[1] += STEP_SIZE_PER_FRAME * dir;

  if (diffZ > 0)
    uniforms->camPos[2] -= STEP_SIZE_PER_FRAME * dir;
  else if (diffZ < 0)
    uniforms->camPos[2] += STEP_SIZE_PER_FRAME * dir;
  printf("MOVING FORWARD %f\n", dir);
}

float3 normalize(float3 a) {
  float d = sqrtf(a.x * a.x + a.y * a.y + a.z * a.z);
  a.x /= d;
  a.y /= d;
  a.z /= d;
  return a;
}

float3 cross(float3 a, float3 b) {
  float3 r = {0};
  r.x = a.y * b.z - a.z * b.y;
  r.y = a.x * b.z - a.z * b.x;
  r.z = a.x * b.y - a.y * b.x;
  return r;
}

void moveLeft(Uniforms *uniforms, float dir) {
  float3 cameraDirection =
      normalize((float3){uniforms->camLookat[0] - uniforms->camPos[0], //
                         uniforms->camLookat[1] - uniforms->camPos[1], //
                         uniforms->camLookat[2] - uniforms->camPos[2]});
  float3 viewportRight =
      cross(cameraDirection, (float3){uniforms->camUp[0], uniforms->camUp[1],
                                      uniforms->camUp[2]});

  uniforms->camPos[0] -= viewportRight.x * STEP_SIZE_PER_FRAME * dir;
  uniforms->camPos[1] -= viewportRight.y * STEP_SIZE_PER_FRAME * dir;
  uniforms->camPos[2] -= viewportRight.z * STEP_SIZE_PER_FRAME * dir;
  printf("MOVING LEFT %f\n", dir);
}

bool update_uniforms(GLFWwindow *window, Uniforms *uniforms) {
  GLFWUserData *ptr = (GLFWUserData *)glfwGetWindowUserPointer(window);

  if (ptr->movingForward != 0) {
    moveForward(uniforms, (float)ptr->movingForward);
    return true;
  }
  if (ptr->movingLeft != 0) {
    moveLeft(uniforms, (float)ptr->movingLeft);
    return true;
  }

  return false;
}
