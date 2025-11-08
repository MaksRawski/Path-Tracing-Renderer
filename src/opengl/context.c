#include "glad/gl.h"
#include "inputs.h"
//
#include "opengl/context.h"

#include <stdio.h>
#include <stdlib.h>

#define UNUSED (void)
// Function to handle glfw errors
void glfw_error_callback(int error, const char *description) {
  fprintf(stderr, "Error (%d): %s\n", error, description);
}

OpenGLContext OpenGLContext_new(const char *window_title, int desired_width,
                                int desired_height) {
  OpenGLContext self = {0};

  glfwSetErrorCallback(glfw_error_callback);

  if (!glfwInit())
    exit(EXIT_FAILURE);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  self.window =
      glfwCreateWindow(desired_width, desired_height, window_title, NULL, NULL);

  if (!self.window) {
    fprintf(stderr, "Failed to create a glfw window!\n");
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(self.window);
  int version = gladLoadGL(glfwGetProcAddress);
  if (version == 0) {
    fprintf(stderr, "Failed to initialize OpenGL context!\n");
    exit(EXIT_FAILURE);
  }
  void *userDataPtr = calloc(1, sizeof(GLFWUserData));
  glfwSetWindowUserPointer(self.window, userDataPtr);

  if (glfwRawMouseMotionSupported())
    glfwSetInputMode(self.window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
  glfwSetInputMode(self.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // set callbacks
  glfwSetCursorPosCallback(self.window, cursor_callback);
  glfwSetCursorEnterCallback(self.window, cursor_enter_callback);
  glfwSetKeyCallback(self.window, key_callback);

  // update the viewport size
  OpenGLContext_get_resolution(&self);

  // Successfully loaded OpenGL
  printf("Loaded OpenGL %d.%d\n", GLAD_VERSION_MAJOR(version),
         GLAD_VERSION_MINOR(version));

  return self;
}

OpenGLResolution OpenGLContext_get_resolution(const OpenGLContext *self) {
  OpenGLResolution res = {0};
  glfwGetFramebufferSize(self->window, &res.width, &res.height);
  // making sure the viewport matches the window size
  glViewport(0, 0, res.width, res.height);
  return res;
}

void OpenGLContext_poll_events(OpenGLContext *self) {
  UNUSED(self);
  glfwPollEvents();
}

void OpenGLContext_swap_buffers(OpenGLContext *self) {
  glfwSwapBuffers(self->window);
}

void OpenGLContext_vsync(OpenGLContext *self, bool enable) {
  UNUSED(self);
  glfwSwapInterval(enable ? 1 : 0);
}

GLFWUserData *OpenGLContext_get_user_data(OpenGLContext *ctx) {
  return (GLFWUserData *)glfwGetWindowUserPointer(ctx->window);
}

void OpenGLContext_delete(OpenGLContext *self) {
  if (self == NULL)
    return;
  glfwTerminate();
  self = NULL;
}
