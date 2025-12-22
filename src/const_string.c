#include "const_string.h"
#include "asserts.h"
#include <stdlib.h>
#include <string.h>

char *copy_string(const char *s) {
  size_t len = strlen(s) + 1;
  char *copy = malloc(len * sizeof(char));
  memcpy(copy, s, len);
  return copy;
}

ConstString ConstString_new_copy(const char *str) {
  ASSERTQ_CUSTOM(str != NULL, "ConstString_new_copy called with NULL!");
  return (ConstString){.str = copy_string(str), ._owned = true};
}

ConstString ConstString_new_with(const char *str) {
  ASSERTQ_CUSTOM(str != NULL, "ConstString_new_with called with NULL!");
  return (ConstString){.str = str, ._owned = false};
}

ConstString ConstString_empty(void) { return ConstString_new_with("\0"); }

bool ConstString_is_empty(ConstString self) {
  return self.str == NULL || *self.str == 0;
}

void ConstString_replace_with(ConstString *self, const char *str) {
  ConstString_delete(self);
  *self = ConstString_new_with(str);
}

void ConstString_replace_copy(ConstString *self, const char *str) {
  ConstString_delete(self);
  *self = ConstString_new_copy(str);
}

void ConstString_delete(ConstString *self) {
  if (self->_owned)
    free((void *)self->str);
  self = NULL;
}
