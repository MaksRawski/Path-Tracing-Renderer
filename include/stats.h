#ifndef STATS_H_
#define STATS_H_

#include <stdbool.h>
#include <stddef.h>

typedef struct {
  // timer's value in seconds of when the rendering was started
  double rendering_start_time;
  // time it took to render all the frames for an image in s
  double rendering_time;
  // timer's value in seconds of when the last frame finished rendering
  double last_frame_end_time;
  // time it took to render last frame in s
  double last_frame_time;
  // when doing progressive rendering means the number of frames that were
  // already taken into account
  unsigned int frame_number;
} Stats;

Stats Stats_default(void);
void Stats_reset(Stats *self);

// recommended buffer size is 16 bytes
bool Stats_string_time(double time_in_s, char *buffer, size_t buf_size);

void Stats_stop_rendering_timer(Stats *self);

#endif // STATS_H_
