#include "./tests_yawpitch.h"
#include "asserts.h"
#include "tests_macros.h"
#include "yawpitch.h"
#include <float.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.141592653589793238462643383279502884
#endif

#define TEST_YAW_PITCH_TO_DIR(_test_name, _yaw, _pitch, _expected_dir,         \
                              _epsilon)                                        \
  bool test_YawPitch_to_dir__##_test_name(void) {                              \
    Vec3d dir = YawPitch_to_dir(YawPitch_new(_yaw, _pitch));                   \
    ASSERT_VEC3D_EQ(dir, Vec3d_new _expected_dir, _epsilon);                   \
    return true;                                                               \
  }

#define TEST_YAW_PITCH_FROM_DIR(_test_name, _dir, _expected_yaw,               \
                                _expected_pitch, _epsilon)                     \
  bool test_YawPitch_from_dir__##_test_name(void) {                            \
    YawPitch yp = YawPitch_from_dir(Vec3d_new _dir);                           \
    ASSERT_EQF(yp.yaw_rad, _expected_yaw, _epsilon);                           \
    ASSERT_EQF(yp.pitch_rad, _expected_pitch, _epsilon);                       \
    return true;                                                               \
  }

#define TEST_YAW_PITCH_ID(_test_name, _yaw, _pitch, _epsilon)                  \
  bool test_YawPitch_id__##_test_name(void) {                                  \
    Vec3d dir = YawPitch_to_dir(YawPitch_new(_yaw, _pitch));                   \
    YawPitch yp = YawPitch_from_dir(dir);                                      \
    ASSERT_EQF(yp.yaw_rad, _yaw, _epsilon);                                    \
    ASSERT_EQF(yp.pitch_rad, _pitch, _epsilon);                                \
    return true;                                                               \
  }

TEST_YAW_PITCH_TO_DIR(by_default_look_at_negative_z_axis, 0, 0, //
                      (0, 0, -1), DBL_EPSILON)
TEST_YAW_PITCH_TO_DIR(yaw_is_counter_clockwise__90_deg, M_PI / 2.0, 0, //
                      (1, 0, 0), DBL_EPSILON)
TEST_YAW_PITCH_TO_DIR(yaw_is_counter_clockwise__60_deg, M_PI / 3.0, 0,
                      (cos(-M_PI / 6.0), 0.0, sin(-M_PI / 6.0)), DBL_EPSILON)

TEST_YAW_PITCH_FROM_DIR(by_default_look_at_negative_z_axis, (0, 0, -1), //
                        0, 0, DBL_EPSILON)
TEST_YAW_PITCH_FROM_DIR(yaw_is_counter_clockwise__90_deg, (1, 0, 0), //
                        M_PI / 2.0, 0, DBL_EPSILON)
TEST_YAW_PITCH_FROM_DIR(yaw_is_counter_clockwise__60_deg,
                        (cos(-M_PI / 6.0), 0.0, sin(-M_PI / 6.0)), //
                        M_PI / 3.0, 0, DBL_EPSILON)

TEST_YAW_PITCH_ID(leet, 1.337, 0.42, DBL_EPSILON)
TEST_YAW_PITCH_ID(decimals, 0.1, 0.2, FLT_EPSILON)
TEST_YAW_PITCH_ID(260deg, 2 * M_PI * 260.0 / 360.0, 0.2, DBL_EPSILON)

TEST_YAW_PITCH_ID(extreme_min, 0.0, -M_PI / 2.0 + 1e-15, DBL_EPSILON)
TEST_YAW_PITCH_ID(extreme_mid, M_PI, 0.0, DBL_EPSILON)
TEST_YAW_PITCH_ID(extreme_max, 2 * M_PI - 1e-5, M_PI / 2.0 - 1e-15,
                  1000 * DBL_EPSILON)

bool all_yawpitch_tests(void) {
  bool ok = true;
  TEST_RUN(test_YawPitch_to_dir__by_default_look_at_negative_z_axis, &ok);
  TEST_RUN(test_YawPitch_to_dir__yaw_is_counter_clockwise__90_deg, &ok);
  TEST_RUN(test_YawPitch_to_dir__yaw_is_counter_clockwise__60_deg, &ok);

  TEST_RUN(test_YawPitch_from_dir__by_default_look_at_negative_z_axis, &ok);
  TEST_RUN(test_YawPitch_from_dir__yaw_is_counter_clockwise__90_deg, &ok);
  TEST_RUN(test_YawPitch_from_dir__yaw_is_counter_clockwise__60_deg, &ok);

  TEST_RUN(test_YawPitch_id__leet, &ok);
  TEST_RUN(test_YawPitch_id__decimals, &ok);
  TEST_RUN(test_YawPitch_id__260deg, &ok);
  TEST_RUN(test_YawPitch_id__extreme_min, &ok);
  TEST_RUN(test_YawPitch_id__extreme_mid, &ok);
  TEST_RUN(test_YawPitch_id__extreme_max, &ok);

  return ok;
}
