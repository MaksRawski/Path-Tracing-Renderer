#ifndef STATS_H_
#define STATS_H_

#include <stdbool.h>
#include <stddef.h>

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
  StatsTimer tlas_build;
  // when doing progressive rendering means the number of frames that were
  // already taken into account
  unsigned int frame_number;
} Stats;

Stats Stats_default(void);
void Stats_reset_rendering(Stats *self);

// recommended buffer size is 16 bytes,
// returns whether the string did fit into the buffer
bool Stats_string_time(double time_in_s, char *buffer, size_t buf_size);

#endif // STATS_H_
