#ifndef ASSERTS_H_
#define ASSERTS_H_
#include "epsilon.h"
#include "stdio.h"  // (s)printf
#include "stdlib.h" // exit

// IDEA: macro to create other versions of macros???

#define ASSERT_CUSTOM(cond, fail_reason)                                       \
  do {                                                                         \
    if (!(cond)) {                                                             \
      printf("%s:%d: \033[31mASSERTION FAILED\033[0m: %s", __FILE__, __LINE__, \
             fail_reason);                                                     \
      return 0;                                                                \
    }                                                                          \
  } while (0);

#define ASSERT_CONDF(cond, val)                                                \
  do {                                                                         \
    char out[128];                                                             \
    sprintf(out, "(%s) %s = %f \n", #cond, #val, val);                         \
    ASSERT_CUSTOM(cond, out);                                                  \
  } while (0);

#define ASSERT_EQF(a, b)                                                       \
  do {                                                                         \
    char out[128];                                                             \
    sprintf(out, "%s (%f) != %s (%f)\n", #a, a, #b, b);                        \
    ASSERT_CUSTOM(is_zero(a - b), out);                                        \
  } while (0);

#define ASSERT_VEC3_EQ(a, b)                                                   \
  do {                                                                         \
    char out[128];                                                             \
    sprintf(out, "%s (%s) != %s (%s)\n", #a, vec3_str(a).s, #b,                \
            vec3_str(b).s);                                                    \
    ASSERT_CUSTOM(vec3_eq(a, b), out);                                         \
  } while (0);

#define TEST_RUN(name)                                                         \
  printf("test %s: \033[33mRUNNING\033[0m\n", #name);                          \
  if (name())                                                                  \
    printf("test %s: \033[32mOK\033[0m\n", #name);                           \
  else                                                                         \
    printf("test %s: \033[31mERROR\033[0m\n", #name);

#define ASSERTQ_CUSTOM(cond, fail_reason)                                      \
  do {                                                                         \
    if (!(cond)) {                                                             \
      printf("%s:%d: \033[31mASSERTION FAILED\033[0m: %s", __FILE__, __LINE__, \
             fail_reason);                                                     \
      exit(1);                                                                 \
    }                                                                          \
  } while (0);

#define ASSERTQ_CONDF(cond, val)                                               \
  do {                                                                         \
    char out[128];                                                             \
    sprintf(out, "(%s) %s = %f \n", #cond, #val, val);                         \
    ASSERTQ_CUSTOM(cond, out);                                                 \
  } while (0);

#define ASSERTQ_EQF(a, b)                                                      \
  do {                                                                         \
    char out[128];                                                             \
    sprintf(out, "%s (%f) != %s (%f)\n", #a, a, #b, b);                        \
    ASSERTQ_CUSTOM(is_zero(a - b), out);                                       \
  } while (0);

#define ASSERTQ_VEC3_EQ(a, b)                                                  \
  do {                                                                         \
    char out[128];                                                             \
    sprintf(out, "%s (%s) != %s (%s)\n", #a, vec3_str(a).s, #b,                \
            vec3_str(b).s);                                                    \
    ASSERTQ(vec3_eq(a, b), out);                                               \
  } while (0);

#endif // ASSERT_H_
