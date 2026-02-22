#ifndef STATS_H_
#define STATS_H_

#include "small_string.h"
#include "tiny_string.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
  double total_time;
  double _start_time, _end_time;
} StatsTimer;

StatsTimer StatsTimer_new(void);
void StatsTimer_start(StatsTimer *self);
void StatsTimer_stop(StatsTimer *self);
double StatsTimer_elapsed(const StatsTimer *self);

typedef struct {
  StatsTimer rendering;
  StatsTimer last_frame_rendering;
  StatsTimer scene_load;
  StatsTimer bvh_build;
  // when doing progressive rendering means the number of frames that were
  // already taken into account
  uint32_t frame_number;
} Stats;

Stats Stats_default(void);
void Stats_reset_rendering(Stats *self);

// recommended buffer size is 16 bytes,
// returns whether the string did fit into the buffer
bool Stats_string_time(double time_in_s, char *buffer, size_t buf_size);

// wrapper around Stats that returns a TinyString or panics if TinyString is too
// small (should be impossible)
TinyString Stats_display(double time_in_s);

SmallString Stats_str(const Stats *self);

#endif // STATS_H_
