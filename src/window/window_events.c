#include "window/window_events.h"

bool WindowEventsData_is_mod_pressed(const WindowEventsData *events, int mod) {
  return events->_mods & mod;
}
