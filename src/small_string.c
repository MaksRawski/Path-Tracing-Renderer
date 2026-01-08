#include "small_string.h"
#include "asserts.h"
#include <string.h>

SmallString SmallString_new(const char *str) {
  ASSERTQ_CUSTOM(str != NULL, "SmallString_new_from called with NULL!");
  SmallString res = {0};
  strncpy(res.str, str, sizeof(res.str));
  return res;
}

bool SmallString_is_empty(const SmallString *self) {
  return self->str[0] == 0;
}
