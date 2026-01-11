#include "test_asserts.h"
#include "asserts.h"
#include "scene/camera.h"
#include "tests_macros.h"
#include <float.h>

bool test_ASSERT_EQF(void) {
  ASSERT_EQF(1.0, 1.0, DBL_EPSILON);
  ASSERT_EQF(1.5, 1.5, DBL_EPSILON);
  ASSERT_EQF(1.0 + DBL_EPSILON, 1.0 + DBL_EPSILON, DBL_EPSILON);
  ASSERT_EQF(1.0 + DBL_EPSILON, 1.0, DBL_EPSILON);
  ASSERT_EQF(M_PI, M_PI, FLT_EPSILON);
  ASSERT_EQF(M_PI - 0.1, M_PI - 0.1, FLT_EPSILON);
  ASSERT_EQF(M_PI - 0.1, M_PI - 0.1, FLT_EPSILON);
  return true;
}

bool all_asserts_tests(void) {
  bool ok = true;
  TEST_RUN(test_ASSERT_EQF, &ok);
  return ok;
}
