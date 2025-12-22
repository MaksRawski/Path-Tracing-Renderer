#ifndef TESTS_MACROS_H_
#define TESTS_MACROS_H_
#include <stdbool.h>

void TESTS_RUN_impl(bool tests_fn(void), const char *section_name);

#define TESTS_RUN(_tests) TESTS_RUN_impl(_tests, #_tests)

void TEST_RUN_impl(bool test_fn(void), const char *test_name, bool *ok);

/* will set ok to false if the test fails */
#define TEST_RUN(_test, ok) TEST_RUN_impl(_test, #_test, ok)

#endif // TESTS_MACROS_H_
