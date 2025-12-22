#include "tests_apply_lut.h"
#include "asserts.h"
#include "scene/bvh.h"
#include "tests_macros.h"

bool test_apply_swaps_lut__id(void) {
  BVHTriCount numbers[] = {0, 1, 2, 3, 4, 5};
  BVHTriCount lut[] = {0, 1, 2, 3, 4, 5};
  BVHTriCount expected[] = {0, 1, 2, 3, 4, 5};

  const unsigned int count = sizeof(lut) / sizeof(BVHTriCount);
  BVH_apply_swaps_lut(lut, numbers, BVHTriCount, count);

  ASSERT_ARRAYN_EQ(numbers, expected, count);
  return true;
}

bool test_apply_swaps_lut__rev(void) {
  BVHTriCount numbers[] = {0, 1, 2, 3, 4, 5};
  BVHTriCount lut[] = {5, 4, 3, 2, 1, 0};
  BVHTriCount expected[] = {5, 4, 3, 2, 1, 0};

  const unsigned int count = sizeof(lut) / sizeof(BVHTriCount);
  BVH_apply_swaps_lut(lut, numbers, BVHTriCount, count);

  ASSERT_ARRAYN_EQ(numbers, expected, count);
  return true;
}

bool test_apply_swaps_lut__random(void) {
  BVHTriCount numbers[] = {0, 1, 2, 3, 4, 5};
  BVHTriCount lut[] = {2, 4, 1, 3, 5, 0};
  BVHTriCount expected[] = {2, 4, 1, 3, 5, 0};
  const unsigned int count = sizeof(lut) / sizeof(BVHTriCount);

  BVH_apply_swaps_lut(lut, numbers, BVHTriCount, count);

  ASSERT_ARRAYN_EQ(numbers, expected, count);
  return true;
}

bool test_apply_swaps_lut__simplest_earlier_breaking_case(void) {
  BVHTriCount numbers[] = {0, 1, 2, 3, 4, 5};
  BVHTriCount lut[] = {2, 0, 1, 3, 4, 5};
  BVHTriCount expected[] = {2, 0, 1, 3, 4, 5};
  const unsigned int count = sizeof(lut) / sizeof(BVHTriCount);

  BVH_apply_swaps_lut(lut, numbers, BVHTriCount, count);

  ASSERT_ARRAYN_EQ(numbers, expected, count);
  return true;
}

bool all_bvh_lut_tests(void) {
  bool ok = true;
  TEST_RUN(test_apply_swaps_lut__id, &ok);
  TEST_RUN(test_apply_swaps_lut__rev, &ok);
  TEST_RUN(test_apply_swaps_lut__random, &ok);
  TEST_RUN(test_apply_swaps_lut__simplest_earlier_breaking_case, &ok);

  return ok;
}
