#include "window/window_events.h"
#include <GLFW/glfw3.h>
#include <stdbool.h>

// key must be a GLFW_KEY_*
bool WindowEventsData_is_key_pressed(const WindowEventsData *events, int key) {
  // "glfwGetKey returns the last state reported for specified key"
  // so even though this looks like polling it actually semantically matches
  // the idea that this structure holds the state
  return glfwGetKey(events->_window, key) == GLFW_PRESS;
}

bool WindowEventsData_is_mod_pressed(const WindowEventsData *events, int mod) {
  return events->_mods & mod;
}

bool WindowEventsData_is_mouse_button_pressed(const WindowEventsData *events,
                                              int key) {
  return glfwGetMouseButton(events->_window, key) == GLFW_PRESS;
}
