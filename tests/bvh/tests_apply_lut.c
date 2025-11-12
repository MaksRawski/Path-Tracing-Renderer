#include "tests_apply_lut.h"
#include "asserts.h"
#include "scene/bvh.h"
#include "tests_macros.h"

bool test_apply_swaps__id(void) {
  BVHTriCount numbers[] = {0, 1, 2, 3, 4, 5};
  BVHTriCount lut[] = {0, 1, 2, 3, 4, 5};
  BVHTriCount expected[] = {0, 1, 2, 3, 4, 5};

  const unsigned int size = sizeof(lut) / sizeof(BVHTriCount);
  BVH_apply_swaps_lut(lut, numbers, BVHTriCount, size);

  ASSERT_ARRAYN_EQ(numbers, expected, BVHTriCount, size);
  return true;
}

bool test_apply_swaps__rev(void) {
  BVHTriCount numbers[] = {0, 1, 2, 3, 4, 5};
  BVHTriCount lut[] = {5, 4, 3, 2, 1, 0};
  BVHTriCount expected[] = {5, 4, 3, 2, 1, 0};

  const unsigned int size = sizeof(lut) / sizeof(BVHTriCount);
  BVH_apply_swaps_lut(lut, numbers, BVHTriCount, size);

  ASSERT_ARRAYN_EQ(numbers, expected, BVHTriCount, size);
  return true;
}

bool test_apply_swaps__random(void) {
  BVHTriCount numbers[] = {0, 1, 2, 3, 4, 5};
  BVHTriCount lut[] = {2, 4, 1, 3, 5, 0};
  BVHTriCount expected[] = {2, 4, 1, 3, 5, 0};

  const unsigned int size = sizeof(lut) / sizeof(BVHTriCount);
  BVH_apply_swaps_lut(lut, numbers, BVHTriCount, size);

  ASSERT_ARRAYN_EQ(numbers, expected, BVHTriCount, size);
  return true;
}

bool all_bvh_lut_tests(void) {
  TEST_RUN(test_apply_swaps__id);
  TEST_RUN(test_apply_swaps__rev);
  TEST_RUN(test_apply_swaps__random);

  return true;
}
