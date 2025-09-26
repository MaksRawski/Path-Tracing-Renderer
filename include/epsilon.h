#ifndef EPSILON_H_
#define EPSILON_H_

#include <stdbool.h>

// yes, this value is chosen kind of arbitrarly since it's just for
// comparing values and how close can they be before considering them equal
#define EPSILON (1.0 / (1 << 20))

static inline bool is_zero(float x) { return x < EPSILON && x > -EPSILON; }

#endif // EPSILON_H_
