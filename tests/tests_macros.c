#include "tests_macros.h"
#include "asserts.h"
#include <stdio.h>

void TESTS_RUN_impl(bool tests_fn(void), const char *section_name, bool *ok) {
  printf("=== %s ===\n", section_name);
  if (tests_fn())
    printf("=== %s: " GREEN("OK") " ===\n\n", section_name);
  else {
    printf("=== %s: " RED("FAILED") " === \n\n", section_name);
    *ok = false;
  }
}

void TEST_RUN_impl(bool test_fn(void), const char *test_name, bool *ok) {
  printf("test %s: " YELLOW("RUNNING") "\n", test_name);
  if (test_fn())
    printf("test %s: " GREEN("OK") "\n", test_name);
  else {
    printf("test %s: " RED("FAILED") "\n", test_name);
    *ok = false;
  }
}
