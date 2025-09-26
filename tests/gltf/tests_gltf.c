#include "tests_gltf.h"
#include "asserts.h"
#include "tests_macros.h"
#include "file_formats/gltf.h"


bool test_load_gltf_scene__cube_camera(void) {
  Scene scene = {0};
  load_gltf_scene(&scene, "tests/gltf/cube-camera.glb");

  ASSERT_EQI(scene.triangles_count, 12);
  ASSERT_EQI(scene.mats_count, 1); // just the default material
  ASSERT_EQI(scene.meshes_count, 1);
  ASSERT_EQI(scene.camera_count, 1);

  return true;
}

bool all_gltf_tests(void) {
  TEST_RUN(test_load_gltf_scene__cube_camera);
  return true;
}
