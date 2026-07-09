#include "small_string.h"
#include <string.h>

SmallString SmallString_new(const char *str) {
  SmallString res = {0};
  if (str[0] != 0)
    strncpy(res.str, str, sizeof(res.str));
  return res;
}

bool SmallString_is_empty(const SmallString *self) {
  return self->str[0] == 0;
}
