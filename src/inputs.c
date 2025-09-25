#include "inputs.h"
#include "renderer.h"
#include "vec3.h"
#include <GLFW/glfw3.h>
#include <math.h>

const float STEP_SIZE_PER_FRAME = 0.05;
const float CURSOR_SENSITIVITY = 0.001;
const float FOCAL_LENGTH = 10.0;
// +Y is UP, must be the same as in the shader
const vec3 UP = {0, 1, 0, 0};
const vec3 DEFAULT_LOOKAT = {0, 0, -1, 0};

#define UNUSED(x) (void)(x)

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
  UNUSED(mods);
  UNUSED(scancode);
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
    if (key == GLFW_KEY_SPACE) {
      userPtr->movingUp = 1;
    } else if (key == GLFW_KEY_C) {
      userPtr->movingUp = -1;
    }
  } else if (action == GLFW_RELEASE) {
    if (key == GLFW_KEY_W || key == GLFW_KEY_S) {
      userPtr->movingForward = 0;
    }
    if (key == GLFW_KEY_A || key == GLFW_KEY_D) {
      userPtr->movingLeft = 0;
    }
    if (key == GLFW_KEY_SPACE || key == GLFW_KEY_C) {
      userPtr->movingUp = 0;
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

void moveUp(RUniforms *uniforms, float dir) {
  /* uniforms->cPos.y += STEP_SIZE_PER_FRAME * dir; */
  uniforms->cPos =
      vec3_add(uniforms->cPos, vec3_mult(UP, STEP_SIZE_PER_FRAME * dir));
}

void moveForward(RUniforms *uniforms, float dir) {
  vec3 camera_dir = vec3_sub(uniforms->cLookat, uniforms->cPos);
  vec3 move_dir = vec3_norm(vec3_mult(camera_dir, dir));
  uniforms->cPos =
      vec3_add(uniforms->cPos, vec3_mult(move_dir, STEP_SIZE_PER_FRAME));
}

void moveLeft(RUniforms *uniforms, float dir) {
  vec3 camera_dir = vec3_sub(uniforms->cLookat, uniforms->cPos);
  vec3 view_left = vec3_norm(vec3_cross(UP, camera_dir));
  vec3 move_dir = vec3_mult(view_left, dir);
  uniforms->cPos =
      vec3_add(uniforms->cPos, vec3_mult(move_dir, STEP_SIZE_PER_FRAME));
}

void cursor_callback(GLFWwindow *window, double xPos, double yPos) {
  GLFWUserData *userPtr = glfwGetWindowUserPointer(window);
  if (userPtr->paused)
    return;
  float x = xPos - userPtr->lastMouseX;
  float y = yPos - userPtr->lastMouseY;

  userPtr->yaw += x * CURSOR_SENSITIVITY;

  float pitch = userPtr->pitch - y * CURSOR_SENSITIVITY;
  if (pitch > -(PI / 2 - 0.05) && pitch < (PI / 2 - 0.05))
    userPtr->pitch = pitch;

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

// updates uniforms with the inputs received from GLFW
bool update_inputs_uniforms(GLFWwindow *window, RUniforms *uniforms) {
  GLFWUserData *ptr = (GLFWUserData *)glfwGetWindowUserPointer(window);

  if (ptr->resetPosition) {
    uniforms->cPos.x = 0.0;
    uniforms->cPos.y = 1;
    uniforms->cPos.z = 0;
    ptr->yaw = 0.0;
    ptr->pitch = 0.0;
    ptr->resetPosition = false;
  }
  if (ptr->releaseCursor) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    ptr->releaseCursor = false;
    ptr->paused = true;
  }

  bool changed = false;
  if (ptr->movingForward != 0) {
    moveForward(uniforms, (float)ptr->movingForward);
    changed = true;
  }
  if (ptr->movingLeft != 0) {
    moveLeft(uniforms, (float)ptr->movingLeft);
    changed = true;
  }
  if (ptr->movingUp != 0) {
    moveUp(uniforms, (float)ptr->movingUp);
    changed = true;
  }

  float lookatX = uniforms->cPos.x + FOCAL_LENGTH * cos(ptr->yaw);
  float lookatZ = uniforms->cPos.z + FOCAL_LENGTH * sin(ptr->yaw);
  float lookatY = uniforms->cPos.y + FOCAL_LENGTH * tan(ptr->pitch);

  changed |= (lookatX != uniforms->cLookat.x ||
              lookatY != uniforms->cLookat.y || uniforms->cLookat.z != lookatZ);

  uniforms->cLookat.x = lookatX;
  uniforms->cLookat.y = lookatY;
  uniforms->cLookat.z = lookatZ;

  return changed;
}
