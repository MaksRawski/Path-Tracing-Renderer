#include "asserts.h"
#include "my_string.h"
#include <stdlib.h>
#include <string.h>

String String_new(unsigned int capacity) {
  String self = {0};
  self.str = calloc(capacity, sizeof(char));
  self.capacity = capacity;
  return self;
}

// Wraps a C-style string into a String object.
String String_of(char *str) {
  ASSERTQ_NOT_NULL(str, "String_of called with NULL!");
  String self = {0};
  self.str = str;
  self.capacity = strlen(str);
  return self;
}

// Creates a new String object with contents copied from the argument.
String String_from(const char *str) {
  ASSERTQ_NOT_NULL(str, "String_from called with NULL!");
  int len = strlen(str);
  String self = String_new(len + 1);
  strncpy(self.str, str, len);
  self.str[len] = '\0';
  return self;
}
