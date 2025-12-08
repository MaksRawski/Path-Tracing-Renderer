#ifndef OPENGL_CONTEXT_H_
#define OPENGL_CONTEXT_H_

#include <GLFW/glfw3.h>
#include <stdbool.h>

#include "opengl/window_coordinate.h"
#include "opengl/window_events.h"

typedef struct {
  GLFWwindow *window;
} OpenGLContext;

typedef struct {
  // set only through poll_events method
  WindowCoordinate last_mouse_pos;
} GLFWUserData;

OpenGLContext OpenGLContext_new(const char *window_title, int width,
                                int height);
GLFWUserData *OpenGLContext_get_user_data(OpenGLContext *self);

OpenGLResolution OpenGLContext_get_window_size(const OpenGLContext *self);
OpenGLResolution OpenGLContext_update_viewport_size(const OpenGLContext *self);
WindowEventsData OpenGLContext_poll_events(OpenGLContext *self);

void OpenGLContext_steal_mouse(GLFWwindow *window);
void OpenGLContext_give_back_mouse(GLFWwindow *window);
void OpenGLContext_swap_buffers(OpenGLContext *self);
void OpenGLContext_vsync(OpenGLContext *self, bool enable);
void OpenGLContext_delete(OpenGLContext *self);

#endif // OPENGL_CONTEXT_H_
