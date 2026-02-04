#include "asserts.h"
#include "glad/gl.h"
//
#include "opengl/context.h"
#include "opengl/gl_call.h"
#include "opengl/resolution.h"
#include "opengl/scaling.h"
#include "opengl/window_coordinate.h"
#include "opengl/window_events.h"

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

// Function to handle glfw errors
void glfw_error_callback(int error, const char *description) {
  fprintf(stderr, "Error (%d): %s\n", error, description);
}

Window Window_new(const char *window_title, int desired_width,
                  int desired_height) {
  Window self = {0};

  glfwSetErrorCallback(glfw_error_callback);

  if (!glfwInit())
    exit(EXIT_FAILURE);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
#ifndef NDEBUG
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  if (glfwPlatformSupported(GLFW_PLATFORM_WIN32))
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WIN32);
  else if (glfwPlatformSupported(GLFW_PLATFORM_COCOA))
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_COCOA);
  else if (glfwPlatformSupported(GLFW_PLATFORM_WAYLAND))
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);
  else if (glfwPlatformSupported(GLFW_PLATFORM_X11))
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);

  self.glfw_window =
      glfwCreateWindow(desired_width, desired_height, window_title, NULL, NULL);

  if (!self.glfw_window) {
    fprintf(stderr, "Failed to create a glfw window!\n");
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(self.glfw_window);
  int version = gladLoadGL(glfwGetProcAddress);
  if (version == 0) {
    fprintf(stderr, "Failed to initialize OpenGL context!\n");
    exit(EXIT_FAILURE);
  }
  void *userDataPtr = calloc(1, sizeof(GLFWUserData));
  glfwSetWindowUserPointer(self.glfw_window, userDataPtr);
  glfwSwapInterval(0); // disable vsync

  // HACK: do initial poll to avoid having huge mouse delta
  Window_poll_events(&self);

  // Successfully loaded OpenGL
  printf("Loaded OpenGL %d.%d\n", GLAD_VERSION_MAJOR(version),
         GLAD_VERSION_MINOR(version));

  return self;
}

OpenGLResolution Window_get_framebuffer_size(const Window *self) {
  OpenGLResolution res = {0};
  glfwGetFramebufferSize(self->glfw_window, (int *)&res.width,
                         (int *)&res.height);

  return res;
}

void Window_display_framebuffer(GLuint fbo, OpenGLResolution fbo_res,
                                OpenGLResolution display_res,
                                OpenGLScalingMode scaling_mode) {
  // desired x and y offset and width and height
  int dx, dy, dw, dh;
  switch (scaling_mode) {
  case OpenGLScalingMode_STRETCH: {
    dx = 0;
    dy = 0;
    dw = display_res.width;
    dh = display_res.height;
    break;
  }
  case OpenGLScalingMode_FIT_CENTER: {
    double fbo_aspect_ratio = (double)fbo_res.width / fbo_res.height;
    double display_aspect_ratio =
        (double)display_res.width / display_res.height;

    if (fbo_aspect_ratio >= display_aspect_ratio) {
      dw = display_res.width;
      dh = display_res.width / fbo_aspect_ratio;
      dx = 0;
      dy = (double)(display_res.height - dh) / 2.0;
    } else {
      dh = display_res.height;
      dw = display_res.height * fbo_aspect_ratio;
      dx = (double)(display_res.width - dw) / 2.0;
      dy = 0;
    }
  } break;
  case OpenGLScalingMode__COUNT: {
    UNREACHABLE();
  }
  }
  GL_CALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo));
  GL_CALL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
  GL_CALL(glBlitFramebuffer(0, 0, fbo_res.width, fbo_res.height, //
                            dx, dy, dx + dw, dy + dh,            //
                            GL_COLOR_BUFFER_BIT, GL_LINEAR));
}

void Window_steal_mouse(GLFWwindow *window) {
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  if (glfwRawMouseMotionSupported())
    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
}

void Window_give_back_mouse(GLFWwindow *window) {
  if (glfwRawMouseMotionSupported())
    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

WindowEventsData Window_poll_events(Window *self) {
  glfwPollEvents();
  GLFWUserData *user_data = Window_get_user_data(self);
  WindowEventsData events = {0};
  events._window = self->glfw_window;
  glfwGetWindowSize(events._window, (int *)&events.window_size.width,
                    (int *)&events.window_size.height);

  // NOTE: user_data->last_mouse_pos would be zero initalized, in which case
  // the first delta will be huge! we assume that the first frame is for
  // settling that
  glfwGetCursorPos(self->glfw_window, &events.mouse_pos.x, &events.mouse_pos.y);
  events.mouse_delta =
      WindowCoordinate_sub(events.mouse_pos, user_data->last_mouse_pos);
  user_data->last_mouse_pos = events.mouse_pos;

  return events;
}

void Window_swap_buffers(Window *self) { glfwSwapBuffers(self->glfw_window); }

GLFWUserData *Window_get_user_data(Window *window) {
  return (GLFWUserData *)glfwGetWindowUserPointer(window->glfw_window);
}

void Window_delete(Window *self) {
  (void)(self);
  glfwTerminate();
}
