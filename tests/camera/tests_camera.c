#include "tests_camera.h"
#include "asserts.h"
#include "scene/camera.h"
#include "tests_macros.h"
#include <float.h>

#include <float.h>

bool test_camera_move_forward(void) {
  Camera cam = Camera_default();
  Camera_move(&cam, (CameraTranslation){.forward = 1, .left = 0, .up = 0}, 1);
  ASSERT_EQF(cam.pos.z, -1, FLT_EPSILON);

  Camera_move(&cam, (CameraTranslation){.forward = 0, .left = 0, .up = 0}, 1);
  ASSERT_EQF(cam.pos.z, -1, FLT_EPSILON);

  Camera_move(&cam, (CameraTranslation){.forward = 1, .left = 0, .up = 0}, 1);
  ASSERT_EQF(cam.pos.z, -2, FLT_EPSILON);

  return true;
}

bool all_camera_tests(void) {
  bool ok = true;
  TEST_RUN(test_camera_move_forward, &ok);
  return ok;
}
