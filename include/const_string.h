#ifndef MY_STRING_H_
#define MY_STRING_H_

#include <stdbool.h>

typedef struct {
  const char *str;
  // whether the string was allocated on the heap
  bool _owned;
} ConstString;

ConstString ConstString_empty(void);

bool ConstString_is_empty(ConstString self);

// Creates a new ConstString "object" with the value COPIED from argument
ConstString ConstString_new_copy(const char *str);

// Creates a new ConstString "object" WITH the given pointer
ConstString ConstString_new_with(const char *str);

// Replaces the value inside ConstString with the value COPIED from argument
void ConstString_replace_copy(ConstString *self, const char *str);

// Replaces the value inside ConstString with the given pointer
void ConstString_replace_with(ConstString *self, const char *str);

void ConstString_delete(ConstString *self);

#endif // MY_STRING_H_
