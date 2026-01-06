#ifndef ASSERTS_H_
#define ASSERTS_H_

#include <stdbool.h>
#include <stdnoreturn.h>

#include "vec3.h"
#include "vec3d.h"

#define RED(_s) "\033[31m" _s "\033[0m"
#define GREEN(_s) "\033[32m" _s "\033[0m"
#define YELLOW(_s) "\033[33m" _s "\033[0m"

void UNUSED_(void *_, ...);
#define UNUSED(...) UNUSED_(NULL, __VA_ARGS__)

noreturn void ERROR_impl(const char *prefix, const char *msg);
noreturn void ERROR_FMT_impl(const char *prefix, const char *fmt, ...);

#define ERROR(_msg) ERROR_impl("ERROR", _msg)
#define ERROR_FMT(_fmt, ...) ERROR_FMT_impl("ERROR", _fmt, __VA_ARGS__)

#define TODO(_msg)                                                             \
  ERROR_FMT_impl(NULL, "%s:%d: " RED("NOT YET IMPLEMENTED") ": " _msg,         \
                 __FILE__, __LINE__)

bool ASSERT_CUSTOM_impl(bool cond, const char *fail_reason,
                        const char *file_name, int line_num);

bool ASSERT_CUSTOM_FMT_impl(bool cond, const char *file_name, int line_num,
                            const char *fmt, ...);

noreturn void UNREACHABLE_impl(const char *file_name, int line_num);

// NOTE: this will NOT be removed from optimized builds as this assumes that
// something must have gone HORRIBLY wrong
#define UNREACHABLE() UNREACHABLE_impl(__FILE__, __LINE__)

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
#define ASSERT_CUSTOM(cond, msg)                                               \
  return_if_not(ASSERT_CUSTOM_impl(cond, msg, __FILE__, __LINE__));

#define ASSERTQ_CUSTOM(cond, msg)                                              \
  exit_if_not(ASSERT_CUSTOM_impl(cond, msg, __FILE__, __LINE__));

#define ASSERT_CUSTOM_FMT(cond, fmt, ...)                                      \
  return_if_not(                                                               \
      ASSERT_CUSTOM_FMT_impl(cond, __FILE__, __LINE__, fmt, __VA_ARGS__))

#define ASSERTQ_CUSTOM_FMT(cond, fmt, ...)                                     \
  exit_if_not(                                                                 \
      ASSERT_CUSTOM_FMT_impl(cond, __FILE__, __LINE__, fmt, __VA_ARGS__))
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
bool ASSERT_CONDF_impl(const char *cond_str, bool cond, const char *val_str,
                       double val, const char *file_name, int line_num);

#define ASSERT_CONDF(cond, val)                                                \
  return_if_not(ASSERT_CONDF_impl(#cond, cond, #val, val, __FILE__, __LINE__));

#define ASSERTQ_CONDF(cond, val)                                               \
  exit_if_not(ASSERT_CONDF_impl(#cond, cond, #val, val, __FILE__, __LINE__));
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
bool ASSERT_EQF_impl(const char *a_str, double a, const char *b_str, double b,
                     const char *file_name, int line_num);

#define ASSERT_EQF(a, b)                                                       \
  return_if_not(ASSERT_EQF_impl(#a, a, #b, b, __FILE__, __LINE__));

#define ASSERTQ_EQF(a, b)                                                      \
  exit_if_not(ASSERT_EQF_impl(#a, a, #b, b, __FILE__, __LINE__));
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
bool ASSERT_EQI_impl(const char *a_str, long a, const char *b_str, long b,
                     const char *file_name, int line_num);

#define ASSERT_EQI(a, b)                                                       \
  return_if_not(ASSERT_EQI_impl(#a, a, #b, b, __FILE__, __LINE__));

#define ASSERTQ_EQI(a, b)                                                      \
  exit_if_not(ASSERT_EQI_impl(#a, a, #b, b, __FILE__, __LINE__));
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
bool ASSERT_VEC3_EQ_impl(const char *a_str, vec3 a, const char *b_str, vec3 b,
                         const char *file_name, int line_num);
#define ASSERT_VEC3_EQ(a, b)                                                   \
  return_if_not(ASSERT_VEC3_EQ_impl(#a, a, #b, b, __FILE__, __LINE__));

#define ASSERTQ_VEC3_EQ(a, b)                                                  \
  exit_if_not(ASSERT_VEC3_EQ_impl(#a, a, #b, b, __FILE__, __LINE__));
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
bool ASSERT_VEC3D_EQ_impl(const char *a_str, Vec3d a, const char *b_str,
                          Vec3d b, const char *file_name, int line_num);
#define ASSERT_VEC3D_EQ(a, b)                                                  \
  return_if_not(ASSERT_VEC3D_EQ_impl(#a, a, #b, b, __FILE__, __LINE__));

#define ASSERTQ_VEC3D_EQ(a, b)                                                 \
  exit_if_not(ASSERT_VEC3D_EQ_impl(#a, a, #b, b, __FILE__, __LINE__));
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// NOTE: it will be hard to refactor sprintfs from this into a function as that
// function signature would have to be generic over the types of array elements!
//
#include "stdio.h"
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
