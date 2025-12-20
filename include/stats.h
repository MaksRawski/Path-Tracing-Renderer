#ifndef STATS_H_
#define STATS_H_

typedef struct {
  // timer's value in seconds of when the last frame finished rendering
  double last_frame_end_time;
  // time it took to render last frame in s
  double last_frame_time;
  // when doing progressive rendering means the number of frames that were
  // already taken into account
  unsigned int frame_number;
} Stats;

Stats Stats_default(void);

#endif // STATS_H_
