#include "tests_bvh_build.h"
#include "asserts.h"
#include "scene.h"
#include "scene/bvh.h"
#include "scene/triangle.h"
#include "tests_macros.h"
#include <stdio.h>
#include <string.h>

#define TRIS_COUNT(_t) sizeof(_t) / sizeof(Triangle)

bool test_bvh_build__basic(void) {
  Triangle tris[1] = {(Triangle){
      .a = vec3_new(0, 0, 0), .b = vec3_new(1, 0, 0), .c = vec3_new(1, 1, 0)}};

  // NOTE: just in case when the function behaves wrong, we don't want it to
  // access other things on the stack, so we allocate way more memory than
  // necessary
  BVHNode nodes[10] = {0};
  BVHTriCount swaps_lut[1] = {0};
  unsigned int nodes_count = 0;

  BVH_build(nodes, &nodes_count, swaps_lut, tris, 0, 1);
  // only root node should have been created
  ASSERT_EQ(nodes_count, 1);
  ASSERT_EQ(nodes[0].first, 0);
  ASSERT_EQ(nodes[0].count, 1);
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

  BVHNode nodes[10] = {0};
  BVHTriCount swaps_lut[1] = {0};
  const unsigned int initial_nodes_offset = 3;
  unsigned int nodes_offset = initial_nodes_offset;
  const unsigned int tris_offset = 1;
  const unsigned int tris_count = 1;

  BVH_build(nodes, &nodes_offset, swaps_lut, tris, tris_offset, tris_count);
  unsigned int nodes_count = nodes_offset - initial_nodes_offset;
  ASSERT_EQ(nodes_count, 1);

  for (unsigned int i = 0; i < sizeof(nodes) / sizeof(BVHNode); ++i) {
    if (i == initial_nodes_offset) {
      ASSERT_EQ(nodes[i].first, 1);
      ASSERT_EQ(nodes[i].count, 1);
      ASSERT_VEC3_EQ(nodes[i].bound_min, vec3_new(0, 0, 0));
      ASSERT_VEC3_EQ(nodes[i].bound_max, vec3_new(1, 1, 0));
    } else {
      // other nodes should be untouched
      ASSERT_EQ(nodes[i].first, 0);
      ASSERT_EQ(nodes[i].count, 0);
      ASSERT_VEC3_EQ(nodes[i].bound_min, vec3_new(0, 0, 0));
      ASSERT_VEC3_EQ(nodes[i].bound_max, vec3_new(0, 0, 0));
    }
  }

  return true;
}

bool test_bvh_build__swaps_lut(void) {
  Triangle tris[] = {(Triangle){.a = vec3_new(-1000, 0, 0),
                                .b = vec3_new(0, -10000, 0),
                                .c = vec3_new(-1000, -1000, 0)},
                     (Triangle){.a = vec3_new(0, 0, 0),
                                .b = vec3_new(1, 0, 0),
                                .c = vec3_new(1, 1, 0)}};

  BVHNode nodes[10] = {0};
  BVHTriCount swaps_lut[1] = {0};
  unsigned int nodes_count = 10;
  unsigned int tris_offset = 1;
  unsigned int tris_count = 1;

  BVH_build(nodes, &nodes_count, swaps_lut, tris, tris_offset, tris_count);
  ASSERT_EQ(swaps_lut[0], tris_offset);

  return true;
}

bool all_bvh_build_tests(void) {
  bool ok = true;
  TEST_RUN(test_bvh_build__basic, &ok);
  TEST_RUN(test_bvh_build__offsets, &ok);
  TEST_RUN(test_bvh_build__swaps_lut, &ok);
  return ok;
}
