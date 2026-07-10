#include "tests_utils.h"
#include "asserts.h"
#include "tests_macros.h"
#include "utils.h"
#include <string.h>


bool test_StringArray_join(void) {
  const char *arr[] = {"The", "quick", "brown", "fox", "jumped", "over", "the", "lazy", "dog"};
  const size_t arr_len = sizeof(arr) / sizeof(arr[0]);
  char buf[256];
  StringArray_join(buf, arr, arr_len, " ");
  ASSERT_EQ(strcmp(buf, "The quick brown fox jumped over the lazy dog"), 0);
  return true;
}

bool all_utils_tests(void) {
  bool ok = true;
  TEST_RUN(test_StringArray_join, &ok);

  return ok;
}
