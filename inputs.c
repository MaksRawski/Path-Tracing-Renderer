#include "inputs.h"
#include "renderer.h"
#include <GLFW/glfw3.h>
#include <math.h>
#include <stdio.h>

const float STEP_SIZE_PER_FRAME = 0.05;
const float CURSOR_SENSITIVITY = 0.1;
const float FOCAL_LENGTH = 10.0;

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
  GLFWUserData *userPtr = glfwGetWindowUserPointer(window);

  if (action == GLFW_PRESS) {
    if (key == GLFW_KEY_W) {
      userPtr->movingForward = 1;
    } else if (key == GLFW_KEY_S) {
      userPtr->movingForward = -1;
    }
    if (key == GLFW_KEY_A) {
      userPtr->movingLeft = 1;
    } else if (key == GLFW_KEY_D) {
      userPtr->movingLeft = -1;
    }
  } else if (action == GLFW_RELEASE) {
    if (key == GLFW_KEY_W) {
      userPtr->movingForward = 0;
    } else if (key == GLFW_KEY_S) {
      userPtr->movingForward = 0;
    }
    if (key == GLFW_KEY_A) {
      userPtr->movingLeft = 0;
    } else if (key == GLFW_KEY_D) {
      userPtr->movingLeft = 0;
    }
    if (key == GLFW_KEY_R) {
      userPtr->resetPosition = true;
    }
    // NOTE: I have caps lock and escape swapped but GLFW takes
    // hardware keys so leaving this hack in so I don't go crazy
    if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_CAPS_LOCK) {
      userPtr->releaseCursor = true;
    }
  }
}

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
}

void cursor_callback(GLFWwindow *window, double xPos, double yPos) {
  GLFWUserData *userPtr = glfwGetWindowUserPointer(window);
  if (userPtr->paused) return;
  float x = xPos - userPtr->lastMouseX;
  float y = yPos - userPtr->lastMouseY;

  userPtr->yawDeg += x * CURSOR_SENSITIVITY;
  if (userPtr->yawDeg >= 360)
    userPtr->yawDeg = userPtr->yawDeg - 360;
  if (userPtr->yawDeg < 0)
    userPtr->yawDeg = 360 - userPtr->yawDeg;

  float pitch = userPtr->pitchDeg - y * CURSOR_SENSITIVITY / 4.;
  if (pitch > -90 && pitch < 90)
    userPtr->pitchDeg = pitch;

  userPtr->lastMouseX = xPos;
  userPtr->lastMouseY = yPos;
}

void cursor_enter_callback(GLFWwindow *window, int entered) {
  GLFWUserData *ptr = (GLFWUserData *)glfwGetWindowUserPointer(window);
  if (entered) {
    // hide cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    ptr->paused = false;
  }
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
  if (ptr->resetPosition) {
    uniforms->camPos[0] = 0.0;
    uniforms->camPos[1] = 1;
    uniforms->camPos[2] = 0;
    ptr->yawDeg = 0.0;
    ptr->pitchDeg = 0.0;
    /* uniforms->camLookat[0] = 0; */
    /* uniforms->camLookat[1] = 1; */
    /* uniforms->camLookat[2] = 0; */
    ptr->resetPosition = false;
  }
  if (ptr->releaseCursor) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    ptr->releaseCursor = false;
    ptr->paused = true;
  }

  float lookatX =
      uniforms->camPos[0] + FOCAL_LENGTH * cos(ptr->yawDeg / 180.0 * PI);
  float lookatZ =
      uniforms->camPos[2] + FOCAL_LENGTH * sin(ptr->yawDeg / 180.0 * PI);
  float lookatY =
      uniforms->camPos[1] + FOCAL_LENGTH * tan(ptr->pitchDeg / 180.0 * PI);

  bool changed =
      (lookatX != uniforms->camLookat[0] || lookatY != uniforms->camLookat[1] ||
       uniforms->camLookat[2] != lookatZ);

  uniforms->camLookat[0] = lookatX;
  uniforms->camLookat[1] = lookatY;
  uniforms->camLookat[2] = lookatZ;
  return changed;
}
