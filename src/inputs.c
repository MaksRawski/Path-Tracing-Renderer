#include "inputs.h"
#include "asserts.h"
#include "renderer.h"
#include "vec3.h"
#include <GLFW/glfw3.h>
#include <math.h>
#include "epsilon.h"

const float STEP_SIZE_PER_FRAME = 0.05;
const float CURSOR_SENSITIVITY = 0.001;

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
  uniforms->cPos = vec3_add(
      uniforms->cPos, vec3_mult(DEFAULT_CAM_UP, STEP_SIZE_PER_FRAME * dir));
}

void moveForward(RUniforms *uniforms, float dir) {
  vec3 camera_dir = vec3_sub(uniforms->cLookat, uniforms->cPos);
  vec3 move_dir = vec3_norm(vec3_mult(camera_dir, dir));
  uniforms->cPos =
      vec3_add(uniforms->cPos, vec3_mult(move_dir, STEP_SIZE_PER_FRAME));
}

void moveLeft(RUniforms *uniforms, float dir) {
  vec3 camera_dir = vec3_sub(uniforms->cLookat, uniforms->cPos);
  vec3 view_left = vec3_norm(vec3_cross(DEFAULT_CAM_UP, camera_dir));
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
  for (int i = 0; i < 10 && userPtr->yaw >= 2 * PI; ++i)
    userPtr->yaw -= 2 * PI;

  for (int i = 0; i < 10 && userPtr->yaw <= 0; ++i)
    userPtr->yaw += 2 * PI;

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
    uniforms->cPos.x = 0;
    uniforms->cPos.y = 0;
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

  vec3 new_lookat = lookat_from_inputs(uniforms->cPos, ptr->yaw, ptr->pitch);
  changed |= !vec3_eq(uniforms->cLookat, new_lookat);
  uniforms->cLookat = new_lookat;

  return changed;
}

// NOTE: these 2 functions must be kept in sync and perform opposite actions
vec3 lookat_from_inputs(vec3 pos, float yaw, float pitch) {
  // yaw is expected to be in range [0, 2*PI] but it'd be ok if it wasn't
  ASSERTQ_CONDF(yaw >= -EPSILON && yaw <= 2 * PI + EPSILON, yaw);
  // this function explodes if pitch is out of exclusive range (-PI/2, PI/2)
  ASSERTQ_CONDF(pitch >= -PI / 2.0 + EPSILON && pitch <= PI / 2.0 - EPSILON,
                pitch);

  // as we want yaw = 0 to correspond to x = 0 and z = -1
  // then regular angle a = 0 <=> yaw = -PI/2
  //
  // TODO: we could optimize it by noticing that this offset corresponds to just
  // doing the other function
  float a = yaw - PI / 2.0;

  return vec3_new(pos.x + cosf(a), pos.y + tanf(pitch), pos.z + sinf(a));
}

YawPitch inputs_from_lookat(vec3 pos, vec3 lookat) {
  vec3 dir = vec3_sub(lookat, pos);
  // the first two should be true, but it's ok if they aren't
  ASSERTQ_CONDF(-1.0 <= dir.x && dir.x <= 1.0, dir.x);
  ASSERTQ_CONDF(-1.0 <= dir.y && dir.y <= 1.0, dir.y);
  // this function explodes if this doesn't hold
  ASSERTQ_CONDF(-1.0 <= dir.z && dir.z <= 1.0, dir.z);

  // yaw = angle - PI / 2
  // a = yaw + PI /2
  // cos a = x
  // sin a = z
  // acos(x) = a
  // asin(z) = a
  //
  // a = asin(z), gives only [-PI/2, PI/2] (i.e. right half) based on the z axis
  // if x > 0 we are on the right half otherwise on the left one

  float a = asinf(dir.z); // [-PI/2, PI/2]
  if (dir.x < 0)
    a = PI - a; // [PI/2, 3/2*PI]

  // a [-PI/2, 3/2*PI]

  float yaw = a + PI / 2.0; // [0, 2*PI]
  ASSERTQ_CONDF(yaw >= -EPSILON && yaw <= 2 * PI + EPSILON, yaw);

  return (YawPitch){.yaw = yaw, .pitch = atanf(dir.y)};
}
