#ifndef WINDOW_H_
#define WINDOW_H_

#include <GLFW/glfw3.h>
#include <stdbool.h>

#include "window/scaling.h"
#include "window/window_coordinate.h"
#include "window/window_events.h"

typedef struct {
  GLFWwindow *glfw_window;
} Window;

typedef struct {
  // NOTE: set only through poll_events method
  WindowCoordinate last_mouse_pos;
} GLFWUserData;

Window Window_new(const char *window_title, int width, int height);

WindowEventsData Window_poll_events(Window *self);

void Window_display_framebuffer(GLuint fbo, WindowResolution fbo_res,
                                WindowResolution display_res,
                                WindowScalingMode scaling_mode);

WindowResolution Window_get_framebuffer_size(const Window *self);

void Window_steal_mouse(GLFWwindow *window);
void Window_give_back_mouse(GLFWwindow *window);
void Window_swap_buffers(Window *self);
void Window_delete(Window *self);

#endif // WINDOW_H_
