#include "bvh/tests_apply_lut.h"
#include "bvh/tests_bvh_build.h"
#include "camera/tests_camera.h"
#include "file_watcher/tests_file_watcher.h"
#include "gltf/tests_gltf.h"
#include "gltf/tests_utils.h"
#include "mat4/tests_mat4.h"
#include "tests_macros.h"
#include "yaw_pitch/tests_yawpitch.h"

int main(void) {
  bool ok = true;
  TESTS_RUN(all_mat4_tests, &ok);
  TESTS_RUN(all_yawpitch_tests, &ok);
  TESTS_RUN(all_gltf_tests, &ok);
  TESTS_RUN(all_gltf_utils_tests, &ok);
  TESTS_RUN(all_bvh_lut_tests, &ok);
  TESTS_RUN(all_bvh_build_tests, &ok);
  TESTS_RUN(all_camera_tests, &ok);
  TESTS_RUN(all_filewatcher_tests, &ok);
  return ok ? 0 : 1;
}
