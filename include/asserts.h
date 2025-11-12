#ifndef ASSERTS_H_
#define ASSERTS_H_

#include <stdbool.h>

#include "vec3.h"
#include "vec3d.h"

// NOTE: message must be a valid identifier
#define STATIC_ASSERT(cond, message_as_identifier)                             \
  typedef char message_as_identifier[2 * (cond) - 1];

bool ASSERT_CUSTOM_impl(bool cond, char *fail_reason, char *file_name,
                        int line_num);

// implementing as a regular function so that stdio.h doesn't have to be loaded
void exit_if_not_impl(bool cond);

#define return_if_not(cond)                                                    \
  if (!cond)                                                                   \
    return false;

// remove ASSERTQ asserts from release builds
#ifdef NDEBUG
#define exit_if_not(cond)
#else
#define exit_if_not(cond) exit_if_not_impl(cond)
#endif

// -----------------------------------------------------------------------------
bool ASSERT_CONDF_impl(char *cond_str, bool cond, char *val_str, double val,
                       char *file_name, int line_num);

#define ASSERT_CONDF(cond, val)                                                \
  return_if_not(ASSERT_CONDF_impl(#cond, cond, #val, val, __FILE__, __LINE__));

#define ASSERTQ_CONDF(cond, val)                                               \
  exit_if_not(ASSERT_CONDF_impl(#cond, cond, #val, val, __FILE__, __LINE__));
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
bool ASSERT_EQF_impl(char *a_str, double a, char *b_str, double b,
                     char *file_name, int line_num);

#define ASSERT_EQF(a, b)                                                       \
  return_if_not(ASSERT_EQF_impl(#a, a, #b, b, __FILE__, __LINE__));

#define ASSERTQ_EQF(a, b)                                                      \
  exit_if_not(ASSERT_EQF_impl(#a, a, #b, b, __FILE__, __LINE__));
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
bool ASSERT_EQI_impl(char *a_str, long a, char *b_str, long b, char *file_name,
                     int line_num);

#define ASSERT_EQI(a, b)                                                       \
  return_if_not(ASSERT_EQI_impl(#a, a, #b, b, __FILE__, __LINE__));

#define ASSERTQ_EQI(a, b)                                                      \
  exit_if_not(ASSERT_EQI_impl(#a, a, #b, b, __FILE__, __LINE__));
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
bool ASSERT_VEC3_EQ_impl(char *a_str, vec3 a, char *b_str, vec3 b,
                         char *file_name, int line_num);
#define ASSERT_VEC3_EQ(a, b)                                                   \
  return_if_not(ASSERT_VEC3_EQ_impl(#a, a, #b, b, __FILE__, __LINE__));

#define ASSERTQ_VEC3_EQ(a, b)                                                  \
  exit_if_not(ASSERT_VEC3_EQ_impl(#a, a, #b, b, __FILE__, __LINE__));
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
bool ASSERT_VEC3D_EQ_impl(char *a_str, Vec3d a, char *b_str, Vec3d b,
                          char *file_name, int line_num);
#define ASSERT_VEC3D_EQ(a, b)                                                  \
  return_if_not(ASSERT_VEC3D_EQ_impl(#a, a, #b, b, __FILE__, __LINE__));

#define ASSERTQ_VEC3D_EQ(a, b)                                                 \
  exit_if_not(ASSERT_VEC3D_EQ_impl(#a, a, #b, b, __FILE__, __LINE__));
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// compare two numeric arrays
#define ASSERT_ARRAYN_EQ(_a, _b, count)                                        \
  do {                                                                         \
    bool arrays_equal = true;                                                  \
    for (unsigned long i = 0; i < count; ++i) {                                \
      if (_a[i] != _b[i]) {                                                    \
        arrays_equal = false;                                                  \
        const char element_fmt[] = _Generic((_a[0]),                           \
            int: "%d",                                                         \
            unsigned int: "%d",                                                \
            long: "%ld",                                                       \
            unsigned long: "%lu");                                             \
        char a_str[20];                                                        \
        char b_str[20];                                                        \
        sprintf(a_str, element_fmt, _a[i]);                                    \
        sprintf(b_str, element_fmt, _b[i]);                                    \
        char msg[256];                                                         \
        sprintf(msg, "%s[%lu] != %s[%lu] (%s != %s)\n", #_a, i, #_b, i, a_str, \
                b_str);                                                        \
        ASSERT_CUSTOM_impl(arrays_equal, msg, __FILE__, __LINE__);             \
      }                                                                        \
      if (!arrays_equal)                                                       \
        return false;                                                          \
    }                                                                          \
  } while (0)
// -----------------------------------------------------------------------------

#endif // ASSERT_H_
