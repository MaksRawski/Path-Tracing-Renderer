#ifndef OPENGL_CONTEXT_H_
#define OPENGL_CONTEXT_H_

#include "opengl/resolution.h"
#include "renderer/user_data.h"
#include <GLFW/glfw3.h>
#include <stdbool.h>

typedef struct {
  GLFWwindow *window;
} OpenGLContext;

typedef struct {
  RendererUserData renderer;
  bool paused;
} GLFWUserData;

OpenGLContext OpenGLContext_new(const char *window_title, int width,
                                int height);
GLFWUserData *OpenGLContext_get_user_data(OpenGLContext *self);
OpenGLResolution OpenGLContext_get_resolution(const OpenGLContext *self);
void OpenGLContext_poll_events(OpenGLContext *self);
void OpenGLContext_swap_buffers(OpenGLContext *self);
void OpenGLContext_vsync(OpenGLContext *self, bool enable);
void OpenGLContext_delete(OpenGLContext *self);

#endif // OPENGL_CONTEXT_H_
