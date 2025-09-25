#ifndef EPSILON_H_
#define EPSILON_H_

#include <stdbool.h>

#define EPSILON (1.0 / (1 << 20))

static inline bool is_zero(float x) {
  return x < EPSILON && x > -EPSILON;
}

#endif // EPSILON_H_
