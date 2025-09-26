#include "./tests_inputs.h"
#include "asserts.h"
#include "inputs.h"
#include "tests_macros.h"
#include <math.h>

bool test_lookat_from_inputs__by_default_look_at_negative_z_axis(void) {
  vec3 pos = vec3_new(0, 0, 0);
  float yaw = 0, pitch = 0;

  vec3 lookat = lookat_from_inputs(pos, yaw, pitch);
  vec3 expected_lookat = vec3_new(0, 0, -1);

  ASSERT_VEC3_EQ(lookat, expected_lookat);
  return true;
}

bool test_lookat_from_inputs__yaw_is_counter_clockwise__90_deg(void) {
  vec3 pos = vec3_new(0, 0, 0);
  float yaw = PI / 2.0, pitch = 0;

  vec3 lookat = lookat_from_inputs(pos, yaw, pitch);
  vec3 expected_lookat = vec3_new(1, 0, 0);

  ASSERT_VEC3_EQ(lookat, expected_lookat);
  return true;
}

bool test_lookat_from_inputs__yaw_is_counter_clockwise__60_deg(void) {
  vec3 pos = vec3_new(0, 0, 0);
  //
  float yaw = PI / 3.0, pitch = 0;

  vec3 lookat = lookat_from_inputs(pos, yaw, pitch);
  vec3 expected_lookat = vec3_new(cosf(-PI / 6.0), 0.0, sinf(-PI / 6.0));

  ASSERT_VEC3_EQ(lookat, expected_lookat);
  return true;
}

bool test_inputs_from_lookat__yaw_is_counter_clockwise__90_deg(void) {
  vec3 pos = vec3_new(0, 0, 0);
  vec3 lookat = vec3_new(1, 0, 0);

  YawPitch yp = inputs_from_lookat(pos, lookat);

  float expected_yaw = PI / 2;
  float expected_pitch = 0;

  ASSERT_EQF(yp.yaw, expected_yaw);
  ASSERT_EQF(yp.pitch, expected_pitch);
  return true;
}

bool test_inputs_from_lookat__yaw_is_counter_clockwise__60_deg(void) {
  // we go counter clockwise 60 deg starting from (0, -1)
  vec3 pos = vec3_new(0, 0, 0);
  vec3 lookat = vec3_new(cosf(-PI / 6.0), 0, sinf(-PI / 6.0));

  YawPitch yp = inputs_from_lookat(pos, lookat);

  float expected_yaw = PI / 3;
  float expected_pitch = 0;

  ASSERT_EQF(yp.yaw, expected_yaw);
  ASSERT_EQF(yp.pitch, expected_pitch);
  return true;
}

bool test_inputs_from_lookat__by_default_look_at_negative_z_axis(void) {
  vec3 pos = vec3_new(0, 3, 0);
  vec3 lookat = vec3_new(0, 3, -1);

  YawPitch yp = inputs_from_lookat(pos, lookat);

  float expected_yaw = 0;
  float expected_pitch = 0;

  ASSERT_EQF(yp.yaw, expected_yaw);
  ASSERT_EQF(yp.pitch, expected_pitch);
  return true;
}

// NOTE: no matter the implementation, these id tests should always pass
bool test_inputs_from_lookat__id(void) {
  vec3 pos = vec3_new(-7, 1.5, -4);
  vec3 lookat = vec3_new(pos.x - sqrtf(2) / 2.0, 1.5, pos.z - sqrtf(2) / 2.0);
  // making sure that that we have a unit vector
  ASSERT_EQF(vec3_mag(vec3_sub(pos, lookat)), 1.0);

  YawPitch yp = inputs_from_lookat(pos, lookat);
  vec3 new_lookat = lookat_from_inputs(pos, yp.yaw, yp.pitch);

  ASSERT_VEC3_EQ(new_lookat, lookat);
  return true;
}

bool test_lookat_from_inputs__id(void) {
  vec3 pos = vec3_new(-7, 1.5, -4);
  float yaw = sqrtf(2.0) / 2.0;
  float pitch = 0.2;

  vec3 lookat = lookat_from_inputs(pos, yaw, pitch);
  YawPitch yp = inputs_from_lookat(pos, lookat);

  ASSERT_EQF(yp.yaw, yaw);
  ASSERT_EQF(yp.pitch, pitch);
  return true;
}

bool test_inputs_from_lookat__real_fail(void) {
  vec3 pos = vec3_new(0, 0, 0);
  vec3 lookat = vec3_new(1 - 0.0340743065, 0, 1 - 0.7411808968);
  // making sure that that we have a unit vector
  ASSERT_EQF(vec3_mag(vec3_sub(pos, lookat)), 1.0);

  YawPitch yp = inputs_from_lookat(pos, lookat);
  vec3 new_lookat = lookat_from_inputs(pos, yp.yaw, yp.pitch);
  ASSERT_EQF(vec3_mag(vec3_sub(pos, new_lookat)), 1.0);

  ASSERT_VEC3_EQ(new_lookat, lookat);
  return true;
}


bool all_inputs_tests(void) {
  TEST_RUN(test_lookat_from_inputs__by_default_look_at_negative_z_axis);
  TEST_RUN(test_lookat_from_inputs__yaw_is_counter_clockwise__90_deg);
  TEST_RUN(test_lookat_from_inputs__yaw_is_counter_clockwise__60_deg);
  TEST_RUN(test_lookat_from_inputs__id);

  TEST_RUN(test_inputs_from_lookat__yaw_is_counter_clockwise__90_deg);
  TEST_RUN(test_inputs_from_lookat__yaw_is_counter_clockwise__60_deg);
  TEST_RUN(test_inputs_from_lookat__by_default_look_at_negative_z_axis);
  TEST_RUN(test_inputs_from_lookat__id);

  TEST_RUN(test_inputs_from_lookat__real_fail);
  return true;
}
