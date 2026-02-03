#ifndef OPENGL_CONTEXT_H_
#define OPENGL_CONTEXT_H_

#include <GLFW/glfw3.h>
#include <stdbool.h>

#include "opengl/scaling.h"
#include "opengl/window_coordinate.h"
#include "opengl/window_events.h"

typedef struct {
  GLFWwindow *glfw_window;
} Window;

typedef struct {
  // NOTE: set only through poll_events method
  WindowCoordinate last_mouse_pos;
} GLFWUserData;

Window Window_new(const char *window_title, int width,
                                int height);
GLFWUserData *Window_get_user_data(Window *self);

OpenGLResolution Window_get_framebuffer_size(const Window *self);
WindowEventsData Window_poll_events(Window *self);

void Window_display_framebuffer(GLuint fbo, OpenGLResolution fbo_res,
                                       OpenGLResolution display_res,
                                       OpenGLScalingMode scaling_mode);

void Window_steal_mouse(GLFWwindow *window);
void Window_give_back_mouse(GLFWwindow *window);
void Window_swap_buffers(Window *self);
void Window_vsync(Window *self, bool enable);
void Window_delete(Window *self);

#endif // OPENGL_CONTEXT_H_
