#include "tests_gltf.h"
#include "asserts.h"
#include "file_formats/gltf.h"
#include "rad_deg.h"
#include "tests_macros.h"
#include <float.h>

bool test_load_gltf_scene__cube_camera(void) {
  Scene scene = {0};
  load_gltf_scene(&scene, "tests/gltf/cube-camera.glb");

  ASSERT_EQ(scene.triangles_count, 12);
  ASSERT_EQ(scene.mats_count, 1); // just the default material
  ASSERT_EQ(scene.meshes_count, 1);

  ASSERT_VEC3_EQ(scene.camera.pos, vec3_new(-7, 0, 0), FLT_EPSILON);
  ASSERT_VEC3_EQ(scene.camera.dir, vec3_new(1, 0, 0), FLT_EPSILON);
  ASSERT_EQF(scene.camera.fov_rad, deg_to_rad(20), FLT_EPSILON);

  ASSERT_RANGE_EX(scene.bvh.nodes_count, 0, 2 * scene.triangles_count);
  ASSERT_VEC3_EQ(scene.bvh.nodes[0].bound_min, vec3_new(-1, -1, -1),
                 FLT_EPSILON);
  ASSERT_VEC3_EQ(scene.bvh.nodes[0].bound_max, vec3_new(1, 1, 1), FLT_EPSILON);

  return true;
}

bool all_gltf_tests(void) {
  bool ok = true;
  TEST_RUN(test_load_gltf_scene__cube_camera, &ok);
  return ok;
}
