#include "asserts.h"
#include "glad/gl.h"
//
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include "window.h"

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

bool g_RENDERER_HOVERED = false;

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

WindowResolution Window_get_framebuffer_size(const Window *self) {
  WindowResolution res = {0};
  glfwGetFramebufferSize(self->glfw_window, (int *)&res.width,
                         (int *)&res.height);

  return res;
}

// TODO: rethink how should display of rendered frame be done if we're using docking
void Window_display_framebuffer(GLuint fbo_tex, WindowResolution fbo_res,
                                WindowResolution _display_res,
                                WindowScalingMode scaling_mode) {
  igBegin("Rendering", NULL, 0);
  g_RENDERER_HOVERED = igIsWindowHovered(0);

  ImVec2 ig_window_size;
  igGetContentRegionAvail(&ig_window_size);
  const struct ImTextureRef *ig_tex_ref = ImTextureRef_ImTextureRef_TextureID(fbo_tex);

  // NOTE: glTF spec actually says:
  // "client implementations SHOULD NOT crop or perform non-uniform scaling (“stretching”) to fill the viewport."
  // so providing stretching feels kind of stupid 
  // desired x and y offset and width and height w.r.t. window size
  switch (scaling_mode) {
  case WindowScalingMode_STRETCH: {
    igImage(*ig_tex_ref, ig_window_size, (ImVec2){.x = 0, .y = 1}, (ImVec2){.x = 1, .y = 0});
    break;
  }
  case WindowScalingMode_FIT_CENTER: {
    const double fbo_aspect_ratio = (double)fbo_res.width / fbo_res.height;
    const double ig_window_aspect_ratio = (double)ig_window_size.x / ig_window_size.y;
    // size that fits inside ig_window_size that stil has the same aspect ratio that fbo_res had
    ImVec2 desired_size;
    if (fbo_aspect_ratio >= ig_window_aspect_ratio) {
      desired_size.x = ig_window_size.x;
      desired_size.y = ig_window_size.x / fbo_aspect_ratio;
    } else {
      desired_size.y = ig_window_size.y;
      desired_size.x = ig_window_size.y * fbo_aspect_ratio;
    }

    // ig_cursor is the top left aligned, left corner in absolute (window) coordinates
    // of where the image would normally be placed
    ImVec2 ig_cursor;
    igGetCursorScreenPos(&ig_cursor);
    // center align
    ig_cursor.x += (ig_window_size.x - desired_size.x) / 2;
    ig_cursor.y += (ig_window_size.y - desired_size.y) / 2;
    igSetCursorScreenPos(ig_cursor);
    igImage(*ig_tex_ref, desired_size, (ImVec2){.x = 0, .y = 1}, (ImVec2){.x = 1, .y = 0});
  } break;
  case WindowScalingMode__COUNT: {
    UNREACHABLE();
  }
  }
  igEnd();
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

static GLFWUserData *Window_get_user_data(Window *window) {
  return (GLFWUserData *)glfwGetWindowUserPointer(window->glfw_window);
}

WindowEventsData Window_poll_events(Window *self) {
  glfwPollEvents();
  GLFWUserData *user_data = Window_get_user_data(self);
  WindowEventsData events = {0};
  glfwGetWindowSize(self->glfw_window, (int *)&events.window_size.width,
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

void Window_delete(Window *self) {
  (void)(self);
  glfwTerminate();
}

// NOTE: key must be a GLFW_KEY_*
bool Window_is_key_pressed(const Window *window, int key) {
  return glfwGetKey(window->glfw_window, key) == GLFW_PRESS;
}

// NOTE: key must be a GLFW_KEY_*
bool Window_is_mouse_button_pressed(const Window *window, int key) {
  return glfwGetMouseButton(window->glfw_window, key) == GLFW_PRESS;
}
