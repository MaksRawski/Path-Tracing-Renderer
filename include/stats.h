#ifndef STATS_H_
#define STATS_H_

#include "small_string.h"
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

SmallString Stats_str(const Stats *self);

#endif // STATS_H_
