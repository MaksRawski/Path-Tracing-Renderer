#include "stats.h"
#include "GLFW/glfw3.h"
#include <stdio.h>

Stats Stats_default(void) { return (Stats){0}; }

void Stats_reset(Stats *self) {
  *self = Stats_default();
  self->rendering_start_time = glfwGetTime();
}

bool Stats_string_time(double time_in_s, char *buffer, size_t buf_size) {
  double time_in_ms = time_in_s * 1000.0;
  double time_in_us = time_in_ms * 1000.0;
  int wanted_to_write;
  if (time_in_ms < 1.0) {
    wanted_to_write = snprintf(buffer, buf_size, "%.3f us", time_in_us);
  } else if (time_in_s < 1.0) {
    wanted_to_write = snprintf(buffer, buf_size, "%.3f ms", time_in_ms);
  } else {
    wanted_to_write = snprintf(buffer, buf_size, "%.3f s", time_in_s);
  }
  if (wanted_to_write >= (long)buf_size) {
    return false;
  }
  return true;
}

void Stats_stop_rendering_timer(Stats *self) {
  if (self->rendering_time == 0)
    self->rendering_time = glfwGetTime() - self->rendering_start_time;
}
