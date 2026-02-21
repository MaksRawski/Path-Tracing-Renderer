#ifndef WINDOW_EVENTS_H_
#define WINDOW_EVENTS_H_

#include "window/resolution.h"
#include "window/window_coordinate.h"

#include <GLFW/glfw3.h>
#include <stdbool.h>

typedef struct {
  // NOTE: mouse_delta on the first frame is equal to mouse_pos
  WindowCoordinate mouse_pos, mouse_delta;
  WindowResolution window_size;
  int _mods;
} WindowEventsData;

bool WindowEventsData_is_mod_pressed(const WindowEventsData *events, int mod);
bool WindowEventsData_is_gui_focused(const WindowEventsData *events);

#endif // WINDOW_EVENTS_H_
