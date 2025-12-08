#include "asserts.h"
#include "epsilon.h"
#include "stdio.h" // (s)printf
#include <stdlib.h>

bool ASSERT_CUSTOM_impl(bool cond, const char *fail_reason,
                        const char *file_name, int line_num) {
  if (!cond) {
    fprintf(stderr, "%s:%d: \033[31mASSERTION FAILED\033[0m: %s", file_name,
            line_num, fail_reason);
    return false;
  }
  return true;
}

void exit_if_not_impl(bool cond) {
  if (!cond)
    exit(1);
}

bool ASSERT_CONDF_impl(const char *cond_str, bool cond, const char *val_str,
                       double val, const char *file_name, int line_num) {
  char out[256];
  sprintf(out, "(%s) %s = %f \n", cond_str, val_str, val);
  return ASSERT_CUSTOM_impl(cond, out, file_name, line_num);
}

bool ASSERT_EQF_impl(const char *a_str, double a, const char *b_str, double b,
                     const char *file_name, int line_num) {
  char out[256];
  sprintf(out, "%s (%f) != %s (%f)\n", a_str, a, b_str, b);
  return ASSERT_CUSTOM_impl(is_zero(a - b), out, file_name, line_num);
}

bool ASSERT_EQI_impl(const char *a_str, long a, const char *b_str, long b,
                     const char *file_name, int line_num) {
  char out[256];
  sprintf(out, "%s (%ld) != %s (%ld)\n", a_str, a, b_str, b);
  return ASSERT_CUSTOM_impl(a == b, out, file_name, line_num);
}

bool ASSERT_VEC3_EQ_impl(const char *a_str, vec3 a, const char *b_str, vec3 b,
                         const char *file_name, int line_num) {
  char out[256];
  sprintf(out, "%s (%s) != %s (%s)\n", a_str, vec3_str(a).s, b_str,
          vec3_str(b).s);
  return ASSERT_CUSTOM_impl(vec3_eq(a, b), out, file_name, line_num);
}

bool ASSERT_VEC3D_EQ_impl(const char *a_str, Vec3d a, const char *b_str,
                          Vec3d b, const char *file_name, int line_num) {
  char out[256];
  sprintf(out, "%s (%s) != %s (%s)\n", a_str, Vec3d_str(a).s, b_str,
          Vec3d_str(b).s);
  return ASSERT_CUSTOM_impl(Vec3d_eq(a, b), out, file_name, line_num);
}
