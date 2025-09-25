#ifndef ASSERTS_H_
#define ASSERTS_H_
#include "stdio.h"

#define ASSERT(cond, fail_reason)                                              \
  do {                                                                         \
    if (!(cond)) {                                                             \
      printf("\033[31mASSERTION FAILED\033[0m at %s:%d %s", __FILE__,          \
             __LINE__, fail_reason);                                           \
      return 0;                                                                \
    }                                                                          \
  } while (0);

#define ASSERT_EQ(a, b)                                                        \
  do {                                                                         \
    char out[128];                                                             \
    sprintf(out, "%s (%f) != %s (%f)\n", #a, a, #b, b);                        \
    ASSERT(a == b, out);                                                       \
  } while (0);

#define ASSERT_VEC3_EQ(a, b)                                                   \
  do {                                                                         \
    char out[128];                                                             \
    sprintf(out, "%s (%s) != %s (%s)\n", #a, vec3_str(a).s, #b,                \
            vec3_str(b).s);                                                    \
    ASSERT(vec3_eq(a, b), out);                                                \
  } while (0);

#define TEST_RUN(name)                                                         \
  printf("\ntest %s: \033[33mRUNNING\033[0m\n", #name);                        \
  if (name())                                                                  \
    printf("test %s: \033[32mOK\033[0m\n", #name);                             \
  else                                                                         \
    printf("test %s: \033[31mERROR\033[0m\n", #name);

#endif // ASSERTS_H_
