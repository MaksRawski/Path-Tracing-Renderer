#include "asserts.h"
#include "inputs.h"

int test_lookat_from_inputs(void) {
  vec3 pos = vec3_new(0, 0, 0);
  float yaw = 0, pitch = 0;

  vec3 lookat = lookat_from_inputs(pos, yaw, pitch);
  vec3 expected_lookat = vec3_new(0, 0, -10);

  ASSERT_VEC3_EQ(lookat, expected_lookat);
  return 1;
}

int test_inputs_from_lookat1(void) {
  vec3 pos = vec3_new(0, 0, 0);
  vec3 lookat = vec3_new(0, 0, -10);

  YawPitch yp = inputs_from_lookat(pos, lookat);
  float expected_yaw = 0;
  float expected_pitch = 0;

  ASSERT_EQ(yp.yaw, expected_yaw);
  ASSERT_EQ(yp.pitch, expected_pitch);
  return 1;
}

int test_inputs_from_lookat2(void) {
  vec3 pos = vec3_new(0, 0, 0);
  vec3 lookat = vec3_new(-10, 0, 0);

  YawPitch yp = inputs_from_lookat(pos, lookat);

  float expected_yaw = -PI / 2;
  float expected_pitch = 0;

  ASSERT_EQ(yp.yaw, expected_yaw);
  ASSERT_EQ(yp.pitch, expected_pitch);
  return 1;
}

int test_inputs_from_lookat3(void) {
  vec3 pos = vec3_new(0, 3, 0);
  vec3 lookat = vec3_new(0, 3, -1);

  YawPitch yp = inputs_from_lookat(pos, lookat);

  float expected_yaw = 0;
  float expected_pitch = 0;

  ASSERT_EQ(yp.yaw, expected_yaw);
  ASSERT_EQ(yp.pitch, expected_pitch);
  return 1;
}

// NOTE: no matter the implementation, this should always pass
int test_inputs_from_pos_lookat_id(void) {
  vec3 pos = vec3_new(0, 0, 0);
  vec3 lookat = vec3_new(0, 0, -10);

  YawPitch yp = inputs_from_lookat(pos, lookat);
  vec3 new_lookat = lookat_from_inputs(pos, yp.yaw, yp.pitch);

  ASSERT_VEC3_EQ(new_lookat, lookat);
  return 1;
}

int main(void) {
  TEST_RUN(test_lookat_from_inputs);
  TEST_RUN(test_inputs_from_lookat1);
  TEST_RUN(test_inputs_from_lookat2);
  TEST_RUN(test_inputs_from_lookat3);
  TEST_RUN(test_inputs_from_pos_lookat_id);

  return 0;
}
