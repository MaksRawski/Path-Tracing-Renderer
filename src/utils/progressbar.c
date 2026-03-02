#include "utils/progressbar.h"
#include "utils.h"
#include <stdio.h>

const char *bar = "****************************************";
const int bar_len = 40;

// completion should be the percentage completion of a task, in range [0,1]
void ProgressBar_print(float completion, double elapsed_time_s) {
  if (completion == 1)
    fprintf(stderr, "\n");
  short lpad = completion * bar_len;
  short rpad = bar_len - lpad;
  fprintf(stderr, "\r[%.*s%*s] %3.1f%% (elapsed: %12s)", lpad, bar, rpad, "",
          completion * 100, Time_format(elapsed_time_s).str);
  fflush(stderr);
}
