#include "asserts.h"
#include "epsilon.h"
#include "stdio.h"
#include <stdarg.h>
#include <stdlib.h>

bool ASSERT_CUSTOM_impl(bool cond, const char *fail_reason,
                        const char *file_name, int line_num) {
  if (!cond) {
    fprintf(stderr, "%s:%d: " RED("ASSERTION FAILED") ": %s\n", file_name,
            line_num, fail_reason);
    return false;
  }
  return true;
}

bool ASSERT_CUSTOM_FMT_impl(bool cond, const char *file_name, int line_num,
                            const char *fmt, ...) {
  char out[1024];
  va_list args;
  va_start(args, fmt);
  vsnprintf(out, sizeof(out), fmt, args);
  va_end(args);
  return ASSERT_CUSTOM_impl(cond, out, file_name, line_num);
}

noreturn void UNREACHABLE_impl(const char *file_name, int line_num) {
  fprintf(stderr, "%s:%d: " RED("UNREACHABLE REACHED!"), file_name, line_num);
  exit(1);
}

noreturn void ERROR_impl(const char *prefix, const char *msg) {
  if (prefix == NULL)
    fprintf(stderr, "%s\n", msg);
  else
    fprintf(stderr, RED("%s") "%s\n", prefix, msg);
  exit(1);
}

noreturn void ERROR_FMT_impl(const char *prefix, const char *fmt, ...) {
  char err_msg[1024];
  va_list args;
  va_start(args, fmt);
  vsnprintf(err_msg, sizeof(err_msg), fmt, args);
  va_end(args);
  ERROR_impl(prefix, err_msg);
}

void exit_if_not_impl(bool cond) {
  if (!cond)
    exit(1);
}

bool ASSERT_CONDF_impl(const char *cond_str, bool cond, const char *val_str,
                       double val, const char *file_name, int line_num) {
  char out[256];
  snprintf(out, sizeof(out), "(%s) %s = %f", cond_str, val_str, val);
  return ASSERT_CUSTOM_impl(cond, out, file_name, line_num);
}

bool ASSERT_EQF_impl(const char *a_str, double a, const char *b_str, double b,
                     const char *file_name, int line_num) {
  char out[256];
  snprintf(out, sizeof(out), "%s (%f) != %s (%f)", a_str, a, b_str, b);
  return ASSERT_CUSTOM_impl(is_zero(a - b), out, file_name, line_num);
}

bool ASSERT_EQI_impl(const char *a_str, long a, const char *b_str, long b,
                     const char *file_name, int line_num) {
  char out[256];
  snprintf(out, sizeof(out), "%s (%ld) != %s (%ld)", a_str, a, b_str, b);
  return ASSERT_CUSTOM_impl(a == b, out, file_name, line_num);
}

bool ASSERT_VEC3_EQ_impl(const char *a_str, vec3 a, const char *b_str, vec3 b,
                         const char *file_name, int line_num) {
  char out[1024];
  snprintf(out, sizeof(out), "%s (%s) != %s (%s)", a_str, vec3_str(a).s, b_str,
           vec3_str(b).s);
  return ASSERT_CUSTOM_impl(vec3_eq(a, b), out, file_name, line_num);
}

bool ASSERT_VEC3D_EQ_impl(const char *a_str, Vec3d a, const char *b_str,
                          Vec3d b, const char *file_name, int line_num) {
  char out[256];
  snprintf(out, sizeof(out), "%s (%s) != %s (%s)", a_str, Vec3d_str(a).s, b_str,
           Vec3d_str(b).s);
  return ASSERT_CUSTOM_impl(Vec3d_eq(a, b), out, file_name, line_num);
}
