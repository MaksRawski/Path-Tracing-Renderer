#include "tests_bvh_build.h"
#include "asserts.h"
#include "scene/bvh.h"
#include "scene/triangle.h"
#include "tests_macros.h"

#define TRIS_LENGTH(_t) sizeof(_t) / sizeof(Triangle)

bool test_bvh_build__basic(void) {
  Triangle tris[1] = {(Triangle){
      .a = vec3_new(0, 0, 0), .b = vec3_new(1, 0, 0), .c = vec3_new(1, 1, 0)}};

  // NOTE: just in case when the function behaves wrong, we don't want it to
  // access other things on the stack, so we allocate way more memory than
  // necessary
  BVHNode nodes[4 * TRIS_LENGTH(tris)] = {0};
  BVHTriCount swaps_lut[TRIS_LENGTH(tris)] = {0};
  unsigned int nodes_count = 0;

  BVH_build(nodes, &nodes_count, swaps_lut, tris, 0, TRIS_LENGTH((tris)));
  // no nodes should have been created
  ASSERT_EQI(nodes_count, 0);
  ASSERT_EQI(nodes[0].count, 1);
  ASSERT_EQI(nodes[0].first, 0);
  ASSERT_VEC3_EQ(nodes[0].bound_min, vec3_new(0, 0, 0));
  ASSERT_VEC3_EQ(nodes[0].bound_max, vec3_new(1, 1, 0));

  return true;
}

bool test_bvh_build__offsets(void) {
  Triangle tris[] = {(Triangle){.a = vec3_new(-1000, 0, 0),
                                .b = vec3_new(0, -10000, 0),
                                .c = vec3_new(-1000, -1000, 0)},
                     (Triangle){.a = vec3_new(0, 0, 0),
                                .b = vec3_new(1, 0, 0),
                                .c = vec3_new(1, 1, 0)}};

  BVHNode nodes[4 * TRIS_LENGTH(tris)] = {0};
  BVHTriCount swaps_lut[TRIS_LENGTH(tris)] = {0};
  unsigned int nodes_count = 10;

  BVH_build(nodes, &nodes_count, swaps_lut, tris, 1, TRIS_LENGTH((tris)));
  ASSERT_EQI(nodes_count, 10);
  ASSERT_EQI(nodes[0].count, 0);
  ASSERT_EQI(nodes[0].first, 0);
  ASSERT_EQI(nodes[1].count, 1);
  ASSERT_EQI(nodes[1].first, 0);
  ASSERT_EQI(nodes[2].count, 0);
  ASSERT_EQI(nodes[2].first, 0);
  ASSERT_VEC3_EQ(nodes[0].bound_min, vec3_new(0, 0, 0));
  ASSERT_VEC3_EQ(nodes[0].bound_max, vec3_new(0, 0, 0));
  ASSERT_VEC3_EQ(nodes[1].bound_min, vec3_new(0, 0, 0));
  ASSERT_VEC3_EQ(nodes[1].bound_max, vec3_new(1, 1, 0));

  return true;
}

bool all_bvh_build_tests(void) {
  bool ok = true;
  TEST_RUN(test_bvh_build__basic, &ok);
  TEST_RUN(test_bvh_build__offsets, &ok);
  return ok;
}
