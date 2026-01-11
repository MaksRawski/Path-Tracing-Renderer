#include "tests_mat4.h"
#include "asserts.h"
#include "mat4.h"
#include "tests_macros.h"

bool test_mat4_trs_inverse__id(void) {
  Mat4 out_mat = {0};
  Mat4 id = {1, 0, 0, 0, //
             0, 1, 0, 0, //
             0, 0, 1, 0, //
             0, 0, 0, 1};
  Mat4_trs_inverse(id, out_mat);
  for (unsigned int i = 0; i < 16; ++i) {
    ASSERT_EQ(out_mat[i], id[i]);
  }
  return true;
}
// Helper: quaternion to 3x3 rotation matrix (column-major)
static void quat_to_mat3(const float q[4], float m[9]) {
  float x = q[0], y = q[1], z = q[2], w = q[3];
  float x2 = x + x, y2 = y + y, z2 = z + z;
  float xx = x * x2, xy = x * y2, xz = x * z2;
  float yy = y * y2, yz = y * z2, zz = z * z2;
  float wx = w * x2, wy = w * y2, wz = w * z2;

  m[0] = 1.0f - (yy + zz);
  m[1] = xy + wz;
  m[2] = xz - wy;

  m[3] = xy - wz;
  m[4] = 1.0f - (xx + zz);
  m[5] = yz + wx;

  m[6] = xz + wy;
  m[7] = yz - wx;
  m[8] = 1.0f - (xx + yy);
}

void Mat4_trs(const float translation[3], const float rotation_quat[4],
              const float scale[3], Mat4 out) {

  float rotation_mat[9];
  quat_to_mat3(rotation_quat, rotation_mat);

  out[0] = rotation_mat[0] * scale[0];
  out[1] = rotation_mat[1] * scale[0];
  out[2] = rotation_mat[2] * scale[0];
  out[3] = 0;

  out[4] = rotation_mat[3] * scale[1];
  out[5] = rotation_mat[4] * scale[1];
  out[6] = rotation_mat[5] * scale[1];
  out[7] = 0;

  out[8] = rotation_mat[6] * scale[2];
  out[9] = rotation_mat[7] * scale[2];
  out[10] = rotation_mat[8] * scale[2];
  out[11] = 0;

  out[12] = translation[0];
  out[13] = translation[1];
  out[14] = translation[2];
  out[15] = 1.0;
}

bool test_mat4_trs_inverse__TRS(void) {
  Mat4 trs = {0};
  Mat4_trs((float[3]){1, 2, 3}, (float[4]){0.5, 0, 0, 0.8660254},
           (float[3]){2, 1.5, 3}, trs);
  Mat4 inverse = {0};
  Mat4_trs_inverse(trs, inverse);

  vec3 out = Mat4_mul_vec3(inverse, Mat4_mul_vec3(trs, vec3_new(1, 1, 1)));

  ASSERT_VEC3_EQ(out, vec3_new(1, 1, 1), FLT_EPSILON);

  return true;
}

bool all_mat4_tests(void) {
  bool ok = true;
  TEST_RUN(test_mat4_trs_inverse__id, &ok);
  TEST_RUN(test_mat4_trs_inverse__TRS, &ok);
  return ok;
}
