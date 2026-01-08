#ifndef SMALL_STRING_H_
#define SMALL_STRING_H_

#include <stdbool.h>

typedef struct {
  char str[1024];
} SmallString;

SmallString SmallString_new(const char *str);

bool SmallString_is_empty(const SmallString *self);

#endif // SMALL_STRING_H_
