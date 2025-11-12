#include "./tests_yawpitch.h"
#include "asserts.h"
#include "tests_macros.h"
#include "yawpitch.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define TEST_YAW_PITCH_TO_DIR(test_name, yaw, pitch, expected_dir)             \
  bool test_YawPitch_to_dir__##test_name(void) {                               \
    Vec3d dir = YawPitch_to_dir(YawPitch_new(yaw, pitch));                     \
    ASSERT_VEC3D_EQ(dir, Vec3d_new expected_dir);                              \
    return true;                                                               \
  }

#define TEST_YAW_PITCH_FROM_DIR(test_name, dir, expected_yaw, expected_pitch)  \
  bool test_YawPitch_from_dir__##test_name(void) {                             \
    YawPitch yp = YawPitch_from_dir(Vec3d_new dir);                            \
    ASSERT_EQF(yp.yaw, expected_yaw);                                          \
    ASSERT_EQF(yp.pitch, expected_pitch);                                      \
    return true;                                                               \
  }

#define TEST_YAW_PITCH_ID(test_name, yaaw, piitch)                             \
  bool test_YawPitch_id__##test_name(void) {                                   \
    Vec3d dir = YawPitch_to_dir(YawPitch_new(yaaw, piitch));                   \
    YawPitch yp = YawPitch_from_dir(dir);                                      \
    ASSERT_EQF(yp.yaw, yaaw);                                                  \
    ASSERT_EQF(yp.pitch, piitch);                                              \
    return true;                                                               \
  }

TEST_YAW_PITCH_TO_DIR(by_default_look_at_negative_z_axis, 0, 0, //
                      (0, 0, -1))
TEST_YAW_PITCH_TO_DIR(yaw_is_counter_clockwise__90_deg, M_PI / 2.0, 0,
                      (1, 0, 0))
TEST_YAW_PITCH_TO_DIR(yaw_is_counter_clockwise__60_deg, M_PI / 3.0, 0,
                      (cosf(-M_PI / 6.0), 0.0, sinf(-M_PI / 6.0)))

TEST_YAW_PITCH_FROM_DIR(by_default_look_at_negative_z_axis, (0, 0, -1), //
                        0, 0)
TEST_YAW_PITCH_FROM_DIR(yaw_is_counter_clockwise__90_deg, (1, 0, 0), //
                        M_PI / 2.0, 0)
TEST_YAW_PITCH_FROM_DIR(yaw_is_counter_clockwise__60_deg,
                        (cosf(-M_PI / 6.0), 0.0, sinf(-M_PI / 6.0)), //
                        M_PI / 3.0, 0)

TEST_YAW_PITCH_ID(leet, 1.337, 0.42)
TEST_YAW_PITCH_ID(decimals, 0.1, 0.2)
TEST_YAW_PITCH_ID(260deg, 2 * M_PI * 260.0 / 360.0, 0.2)

TEST_YAW_PITCH_ID(extreme_min, 0.0, -M_PI / 2.0 + 1e-15)
TEST_YAW_PITCH_ID(extreme_mid, M_PI, 0.0)
TEST_YAW_PITCH_ID(extreme_max, 2 * M_PI - 1e-5, M_PI / 2.0 - 1e-15)

bool all_yawpitch_tests(void) {
  TEST_RUN(test_YawPitch_to_dir__by_default_look_at_negative_z_axis);
  TEST_RUN(test_YawPitch_to_dir__yaw_is_counter_clockwise__90_deg);
  TEST_RUN(test_YawPitch_to_dir__yaw_is_counter_clockwise__60_deg);

  TEST_RUN(test_YawPitch_from_dir__by_default_look_at_negative_z_axis);
  TEST_RUN(test_YawPitch_from_dir__yaw_is_counter_clockwise__90_deg);
  TEST_RUN(test_YawPitch_from_dir__yaw_is_counter_clockwise__60_deg);

  TEST_RUN(test_YawPitch_id__leet);
  TEST_RUN(test_YawPitch_id__decimals);
  TEST_RUN(test_YawPitch_id__260deg);
  TEST_RUN(test_YawPitch_id__extreme_min);
  TEST_RUN(test_YawPitch_id__extreme_mid);
  TEST_RUN(test_YawPitch_id__extreme_max);

  return true;
}
