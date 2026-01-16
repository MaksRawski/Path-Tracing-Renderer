#ifndef EPSILON_H_
#define EPSILON_H_

#include <float.h>
#include <math.h>
#include <stdbool.h>

static inline bool is_zerof(float x) {
  return -FLT_EPSILON < x && x < FLT_EPSILON;
}

static inline bool is_zero(double x) {
  return -DBL_EPSILON < x && x < DBL_EPSILON;
}

static inline bool double_equal(double a, double b, double relative_epsilon) {
  return fabs(a - b) <=
         ((fabs(a) > fabs(b) ? fabs(a) : fabs(b)) * relative_epsilon);
}

static inline bool float_equal(float a, float b, float relative_epsilon) {
  return fabsf(a - b) <=
         ((fabsf(a) > fabsf(b) ? fabsf(b) : fabsf(a)) * relative_epsilon);
}

#endif // EPSILON_H_
