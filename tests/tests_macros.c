#include "tests_macros.h"
#include <stdio.h>

void TESTS_RUN_impl(bool tests_fn(void), const char *section_name) {
  printf("=== %s ===\n", section_name);
  if (tests_fn())
    printf("=== %s: \033[32mOK\033[0m ===\n\n", section_name);
  else
    printf("=== %s: \033[31mERROR\033[0m === \n\n", section_name);
}

void TEST_RUN_impl(bool test_fn(void), const char *test_name, bool *ok) {
  printf("test %s: \033[33mRUNNING\033[0m\n", test_name);
  if (test_fn())
    printf("test %s: \033[32mOK\033[0m\n", test_name);
  else {
    printf("test %s: \033[31mERROR\033[0m\n", test_name);
    *ok = false;
  }
}
