#ifndef ASSERTS_H_
#define ASSERTS_H_
#include "vec3.h"

int ASSERT_CUSTOM_impl(bool cond, char *fail_reason, char *file_name,
                       int line_num);

#define return_if(cond)                                                        \
  if (cond)                                                                    \
    return 0;

// implementing as a regular function so that stdio.h doesn't have to be loaded
void exit_if(bool cond);

// -----------------------------------------------------------------------------
int ASSERT_CONDF_impl(char *cond_str, bool cond, char *val_str, float val,
                      char *file_name, int line_num);

#define ASSERT_CONDF(cond, val)                                                \
  return_if(ASSERT_CONDF_impl(#cond, cond, #val, val, __FILE__, __LINE__));

#define ASSERTQ_CONDF(cond, val)                                               \
  exit_if(ASSERT_CONDF_impl(#cond, cond, #val, val, __FILE__, __LINE__));
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
int ASSERT_EQF_impl(char *a_str, float a, char *b_str, float b, char *file_name,
                    int line_num);

#define ASSERT_EQF(a, b)                                                       \
  return_if(ASSERT_EQF_impl(#a, a, #b, b, __FILE__, __LINE__));

#define ASSERTQ_EQF(a, b)                                                      \
  exit_if(ASSERT_EQF_impl(#a, a, #b, b, __FILE__, __LINE__));
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
int ASSERT_VEC3_EQ_impl(char *a_str, vec3 a, char *b_str, vec3 b,
                        char *file_name, int line_num);
#define ASSERT_VEC3_EQ(a, b)                                                   \
  return_if(ASSERT_VEC3_EQ_impl(#a, a, #b, b, __FILE__, __LINE__));

#define ASSERTQ_VEC3_EQ(a, b)                                                  \
  exit_if(ASSERT_VEC3_EQ_impl(#a, a, #b, b, __FILE__, __LINE__));
// -----------------------------------------------------------------------------

#endif // ASSERT_H_
