#include "asserts.h"
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
  char fail_reason[1024];
  va_list args;
  va_start(args, fmt);
  vsnprintf(fail_reason, sizeof(fail_reason), fmt, args);
  va_end(args);

  return ASSERT_CUSTOM_impl(cond, fail_reason, file_name, line_num);
}

noreturn void UNREACHABLE_impl(const char *file_name, int line_num) {
  fprintf(stderr, "%s:%d: " RED("UNREACHABLE REACHED!"), file_name, line_num);
  exit(1);
}

noreturn void ERROR_impl(const char *prefix, const char *msg) {
  if (prefix == NULL)
    fprintf(stderr, "%s\n", msg);
  else
    fprintf(stderr, RED("%s") ": %s\n", prefix, msg);
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

bool ASSERT_EQ_impl(bool equal, const char *a_name, const char *b_name,
                    const char *file_name, int line_num,
                    const char *single_val_fmt, ...) {
  char fmt_str[512];
  snprintf(fmt_str, sizeof(fmt_str), "%s (%s) != %s (%s)", a_name,
           single_val_fmt, b_name, single_val_fmt);

  char out[1024];
  va_list args;
  va_start(args, single_val_fmt);
  vsnprintf(out, sizeof(out), fmt_str, args);
  va_end(args);

  return ASSERT_CUSTOM_impl(equal, out, file_name, line_num);
}
