#ifndef MY_STRING_H_
#define MY_STRING_H_

typedef struct {
  char *str;
  unsigned long capacity;
} String;

String String_new(unsigned int capacity);
String String_of(char *str);
String String_from(const char *str);

#endif // MY_STRING_H_
