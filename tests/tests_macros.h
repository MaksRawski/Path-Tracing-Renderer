#ifndef TESTS_MACROS_H_
#define TESTS_MACROS_H_
#include "stdio.h"

#define TEST_RUN(name)                                                         \
  printf("test %s: \033[33mRUNNING\033[0m\n", #name);                          \
  if (name())                                                                  \
    printf("test %s: \033[32mOK\033[0m\n\n", #name);                             \
  else                                                                         \
    printf("test %s: \033[31mERROR\033[0m\n\n", #name);

#endif // TESTS_MACROS_H_
