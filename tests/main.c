#include "gltf/tests_gltf.h"
#include "yaw_pitch/tests_yawpitch.h"
#include "tests_macros.h"

int main(void) {
  TEST_RUN(all_yawpitch_tests);
  TEST_RUN(all_gltf_tests);
  return 0;
}
