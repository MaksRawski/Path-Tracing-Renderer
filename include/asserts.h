#ifndef ASSERTS_H_
#define ASSERTS_H_

#include "arena.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdnoreturn.h>

#define RED(_s) "\033[31m" _s "\033[0m"
#define GREEN(_s) "\033[32m" _s "\033[0m"
#define YELLOW(_s) "\033[33m" _s "\033[0m"

static inline void UNUSED_(void *_, ...) {}
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

// NOTE: constructs a format string literal with _pre before and _post after the
// appropriate format specifier for a given _val
#define GENERIC_FMT_STRING(_pre, _val, _post)                                  \
  _Generic((_val),                                                             \
      _Bool: _pre "%d" _post,                                                  \
      unsigned int: _pre "%d" _post,                                           \
      int: _pre "%d" _post,                                                    \
      float: _pre "%f" _post,                                                  \
      double: _pre "%f" _post,                                                 \
      unsigned long: _pre "%lu" _post,                                         \
      long: _pre "%ld" _post,                                                  \
      char *: _pre "%s" _post)

static const char *_GENERIC_FMT_STRING__EXAMPLE =
    GENERIC_FMT_STRING("abc ", 1.337, " def"); // equivalent to abc %f def

// -----------------------------------------------------------------------------
#define ASSERT_CUSTOM_FMT_(_cond, _fmt, ...)                                   \
  ASSERT_CUSTOM_FMT_impl(_cond, __FILE__, __LINE__, _fmt, __VA_ARGS__)

#define ASSERT_CUSTOM_FMT(_cond, _fmt, ...)                                    \
  return_if_not(ASSERT_CUSTOM_FMT_(_cond, _fmt, __VA_ARGS__))

#define ASSERTQ_CUSTOM_FMT(_cond, _fmt, ...)                                   \
  exit_if_not(ASSERT_CUSTOM_FMT_(_cond, _fmt, __VA_ARGS__))

#define ASSERT_CUSTOM(_cond, _msg)                                             \
  return_if_not(ASSERT_CUSTOM_FMT_(_cond, _msg "%0.0f", 0))

#define ASSERTQ_CUSTOM(_cond, _msg)                                            \
  exit_if_not(ASSERT_CUSTOM_FMT_(_cond, _msg "%0.0f", 0))

static inline void test_ASSERT_CUSTOM_FMT(void) {
  bool test_cond = true;
  ASSERT_CUSTOM_FMT_(test_cond, "fail reason fmt string %s %s", "1st value",
                     "2nd value");
}
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
#define ASSERT_COND_(_cond, _val)                                              \
  ASSERT_CUSTOM_FMT_(_cond, GENERIC_FMT_STRING("%s (%s = ", _val, ")"),        \
                     #_cond, #_val, _val)

#define ASSERT_COND(_cond, _val) return_if_not(ASSERT_COND_(_cond, _val))

#define ASSERTQ_COND(_cond, _val) exit_if_not(ASSERT_COND_(_cond, _val))

static inline void test_ASSERT_COND(void) {
  int x = 2;
  ASSERTQ_COND(x < 10, x); // will exit if x fails the condition
}
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
bool ASSERT_EQ_impl(bool equal, const char *a_name, const char *b_name,
                    const char *file_name, int line_num,
                    const char *single_val_fmt, ...);

#define ASSERT_EQF_(_a, _b, _epsilon)                                          \
  ASSERT_EQ_impl(-(_epsilon) < (_a - _b) && (_a - _b) < (_epsilon), #_a, #_b,  \
                 __FILE__, __LINE__, "%f", _a, _b)

#define ASSERT_EQF(_a, _b, _epsilon)                                           \
  return_if_not(ASSERT_EQF_(_a, _b, _epsilon));

#define ASSERTQ_EQF(_a, _b, _epsilon)                                          \
  exit_if_not(ASSERT_EQF_(_a, _b, _epsilon));

#define ASSERT_EQ_(_a, _b)                                                     \
  ASSERT_EQ_impl(_a == _b, #_a, #_b, __FILE__, __LINE__,                       \
                 GENERIC_FMT_STRING("", _a, ""), _a, _b)

#define ASSERT_EQ(_a, _b) return_if_not(ASSERT_EQ_(_a, _b));
#define ASSERTQ_EQ(_a, _b) exit_if_not(ASSERT_EQ_(_a, _b));

// NOTE: functions _type_eq and _type_str must exist! _type_str should return a
// SmallString
#define ASSERT_TYPE_EQ_(_type, _a, _b)                                         \
  ASSERT_EQ_impl(_type##_eq(_a, _b), #_a, #_b, __FILE__, __LINE__, "%s",       \
                 _type##_str(_a).str, _type##_str(_b).str)

#define ASSERT_VEC3_EQ(_a, _b) return_if_not(ASSERT_TYPE_EQ_(vec3, _a, _b));
#define ASSERTQ_VEC3_EQ(_a, _b) exit_if_not(ASSERT_TYPE_EQ_(vec3, _a, _b));

#define ASSERT_VEC3D_EQ(_a, _b) return_if_not(ASSERT_TYPE_EQ_(Vec3d, _a, _b));
#define ASSERTQ_VEC3D_EQ(_a, _b) exit_if_not(ASSERT_TYPE_EQ_(Vec3d, _a, _b));

// -----------------------------------------------------------------------------

#endif // ASSERT_H_
