#ifndef EPSILON_H_
#define EPSILON_H_

#include <float.h>
#include <math.h>
#include <stdbool.h>

static inline bool is_zero(double x, double epsilon) {
  return fabs(x) < epsilon;
}

static inline bool is_zerof(float x, float epsilon) {
  return fabsf(x) < epsilon;
}

static inline bool double_equal_expected(double v, double expected,
                                         double epsilon) {
  return fabs(v - expected) <=
         (fabs(expected) > 1.0 ? fabs(expected) * epsilon : epsilon);
}

static inline bool float_equal_expected(float v, float expected,
                                        float epsilon) {
  return fabsf(v - expected) <=
         (fabsf(expected) > 1.0 ? fabsf(expected) * epsilon : epsilon);
}

static inline bool double_equal(double a, double b, double relative_epsilon) {
  return fabs(a - b) <=
         ((fabs(a) > fabs(b) ? fabs(a) : fabs(b)) * relative_epsilon);
}

static inline bool float_equal(float a, float b, float relative_epsilon) {
  return fabsf(a - b) <=
         ((fabsf(a) > fabsf(b) ? fabsf(a) : fabsf(b)) * relative_epsilon);
}

#endif // EPSILON_H_
