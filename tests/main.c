#include "bvh/tests_apply_lut.h"
#include "camera/tests_camera.h"
#include "file_watcher/tests_file_watcher.h"
#include "gltf/tests_gltf.h"
#include "tests_macros.h"
#include "yaw_pitch/tests_yawpitch.h"
#include "bvh/tests_bvh_build.h"

int main(void) {
  TESTS_RUN(all_yawpitch_tests);
  TESTS_RUN(all_gltf_tests);
  TESTS_RUN(all_bvh_lut_tests);
  TESTS_RUN(all_bvh_build_tests);
  TESTS_RUN(all_camera_tests);
  TESTS_RUN(all_filewatcher_tests);
  return 0;
}
