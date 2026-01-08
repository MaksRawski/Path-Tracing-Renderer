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

#define BIG_EPSILON 10 * FLT_EPSILON

static inline bool double_equal(double a, double b, double epsilon) {
  return fabs(a - b) <= ((fabs(a) > fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}

#endif // EPSILON_H_
