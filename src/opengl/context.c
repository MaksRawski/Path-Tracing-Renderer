#include "glad/gl.h"
//
#include "opengl/context.h"
#include "opengl/window_coordinate.h"
#include "opengl/window_events.h"

#include <GLFW/glfw3.h>
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

  if (glfwPlatformSupported(GLFW_PLATFORM_WIN32))
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WIN32);
  else if (glfwPlatformSupported(GLFW_PLATFORM_COCOA))
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_COCOA);
  else if (glfwPlatformSupported(GLFW_PLATFORM_WAYLAND))
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);
  else if (glfwPlatformSupported(GLFW_PLATFORM_X11))
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);

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

  // update the viewport size
  OpenGLContext_update_viewport_size(&self);

  // HACK: do initial poll to avoid having huge mouse delta
  OpenGLContext_poll_events(&self);

  // Successfully loaded OpenGL
  printf("Loaded OpenGL %d.%d\n", GLAD_VERSION_MAJOR(version),
         GLAD_VERSION_MINOR(version));

  return self;
}

// returns the framebuffer resolution and updates with it glViewport
OpenGLResolution OpenGLContext_get_window_size(const OpenGLContext *self) {
  OpenGLResolution res = {0};
  glfwGetWindowSize(self->window, (int *)&res.width, (int *)&res.height);
  // update viewport size
  OpenGLContext_update_viewport_size(self);

  return res;
}

// returns the framebuffer resolution and updates with it glViewport
// NOTE: this isn't necessarily window size, as it considers scaling
OpenGLResolution OpenGLContext_update_viewport_size(const OpenGLContext *self) {
  OpenGLResolution res = {0};
  glfwGetFramebufferSize(self->window, (int *)&res.width, (int *)&res.height);
  // making sure the viewport matches the framebuffer size
  glViewport(0, 0, res.width, res.height);
  return res;
}

void OpenGLContext_steal_mouse(GLFWwindow *window) {
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  if (glfwRawMouseMotionSupported())
    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
}

void OpenGLContext_give_back_mouse(GLFWwindow *window) {
  if (glfwRawMouseMotionSupported())
    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

WindowEventsData OpenGLContext_poll_events(OpenGLContext *self) {
  glfwPollEvents();
  GLFWUserData *user_data = OpenGLContext_get_user_data(self);
  WindowEventsData events = {0};
  events._window = self->window;
  glfwGetWindowSize(events._window, (int *)&events.window_size.width,
                    (int *)&events.window_size.height);

  // NOTE: user_data->last_mouse_pos would be zero initalized, in which case
  // the first delta will be huge! we assume that the first frame is for
  // settling that
  glfwGetCursorPos(self->window, &events.mouse_pos.x, &events.mouse_pos.y);
  events.mouse_delta =
      WindowCoordinate_sub(events.mouse_pos, user_data->last_mouse_pos);
  user_data->last_mouse_pos = events.mouse_pos;

  // update the viewport size
  OpenGLContext_update_viewport_size(self);

  return events;
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
