#include "stats.h"
#include "GLFW/glfw3.h"
#include "asserts.h"
#include <stdio.h>

StatsTimer StatsTimer_new(void) { return (StatsTimer){0}; }
void StatsTimer_start(StatsTimer *self) {
  self->_start_time = glfwGetTime();
  self->_end_time = 0;
  self->total_time = 0;
}
void StatsTimer_stop(StatsTimer *self) {
  // if the timer wasn't started or already was stopped
  if (self->_start_time == 0 || self->total_time != 0)
    return;
  self->_end_time = glfwGetTime();
  self->total_time = self->_end_time - self->_start_time;
}
double StatsTimer_elapsed(const StatsTimer *self) {
  // if the timer wasn't started
  if (self->_start_time == 0)
    return 0;
  return glfwGetTime() - self->_start_time;
}

Stats Stats_default(void) {
  Stats stats = {0};
  return stats;
}

void Stats_reset_rendering(Stats *self) {
  self->last_frame_rendering = StatsTimer_new();
  self->rendering = StatsTimer_new();
  self->frame_number = 0;
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

TinyString Stats_display(double time_in_s) {
  TinyString out = {0};
  ASSERTQ_CUSTOM(Stats_string_time(time_in_s, out.str, sizeof(out)),
                 "TinyString turned out to be too tiny for `Stats_string_time` "
                 "formatting!");
  return out;
}

SmallString Stats_str(const Stats *self) {
  SmallString out = {0};
  int written =
      snprintf(out.str, sizeof(out.str),
               "scene load time: %s\nbvh build time: %s\nrendering time: %s\n",
               Stats_display(self->scene_load.total_time).str,
               Stats_display(self->bvh_build.total_time).str,
               Stats_display(self->rendering.total_time).str);
  ASSERTQ_CUSTOM(written < (int)sizeof(out.str),
                 "SmallString turned out to be too small for Stats");

  return out;
}
