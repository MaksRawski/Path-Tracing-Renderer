#include "tests_gltf.h"
#include "asserts.h"
#include "file_formats/gltf.h"
#include "rad_deg.h"
#include "tests_macros.h"

bool test_load_gltf_scene__cube_camera(void) {
  Scene scene = {0};
  load_gltf_scene(&scene, "tests/gltf/cube-camera.glb");

  ASSERT_EQI(scene.triangles_count, 12);
  ASSERT_EQI(scene.mats_count, 1); // just the default material
  ASSERT_EQI(scene.meshes_count, 1);
  ASSERT_EQI(scene.mesh_primitives_count, 1);
  ASSERT_EQI(scene.mesh_instances_count, 1);

  ASSERT_VEC3_EQ(scene.camera.pos, vec3_new(-3.8, 0, 0));
  ASSERT_VEC3_EQ(scene.camera.dir, vec3_new(1, 0, 0));
  ASSERT_EQF(scene.camera.fov_rad, deg_to_rad(39.6));

  ASSERT_CONDF(0 < scene.bvh_nodes_count &&
                   scene.bvh_nodes_count < scene.triangles_count,
               scene.bvh_nodes_count);
  ASSERT_VEC3_EQ(scene.bvh_nodes[0].bound_min, vec3_new(-1, -1, -1));
  ASSERT_VEC3_EQ(scene.bvh_nodes[0].bound_max, vec3_new(1, 1, 1));

  return true;
}

bool all_gltf_tests(void) {
  bool ok = true;
  TEST_RUN(test_load_gltf_scene__cube_camera, &ok);
  return ok;
}
