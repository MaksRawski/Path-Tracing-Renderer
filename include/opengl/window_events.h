#ifndef WINDOW_EVENTS_H_
#define WINDOW_EVENTS_H_

#include "opengl/resolution.h"
#include "opengl/window_coordinate.h"

#include <GLFW/glfw3.h>
#include <stdbool.h>

typedef struct {
  // NOTE: mouse_delta on the first frame is equal to mouse_pos
  WindowCoordinate mouse_pos, mouse_delta;
  OpenGLResolution window_size;
  // HACK: to have is_key_pressed method
  GLFWwindow *_window;
  int _mods;
} WindowEventsData;

bool WindowEventsData_is_key_pressed(const WindowEventsData *events, int key);
bool WindowEventsData_is_mod_pressed(const WindowEventsData *events, int mod);
bool WindowEventsData_is_mouse_button_pressed(const WindowEventsData *events, int key);
bool WindowEventsData_is_gui_focused(const WindowEventsData *events);

#endif // WINDOW_EVENTS_H_
