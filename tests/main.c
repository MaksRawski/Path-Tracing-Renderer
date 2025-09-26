#include "gltf/tests_gltf.h"
#include "inputs/tests_inputs.h"
#include "tests_macros.h"

int main(void) {
  TEST_RUN(all_inputs_tests);
  TEST_RUN(all_gltf_tests);
  return 0;
}
