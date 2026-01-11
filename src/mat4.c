#include "mat4.h"
#include <math.h>

// assumes a column-major matrix, also applies translation
vec3 Mat4_mul_vec3(const Mat4 mat, const vec3 vec) {
  vec3 v = {0};
  v.x = mat[0] * vec.x + mat[4] * vec.y + mat[8] * vec.z + mat[12] * 1;
  v.y = mat[1] * vec.x + mat[5] * vec.y + mat[9] * vec.z + mat[13] * 1;
  v.z = mat[2] * vec.x + mat[6] * vec.y + mat[10] * vec.z + mat[14] * 1;

  return v;
}

// NOTE: returns Identity matrix in case of a non-invertible matrix
void Mat4_trs_inverse(const Mat4 mat, Mat4 out_inverse) {
  float det = mat[0] * (mat[5] * mat[10] - mat[6] * mat[9]) -
              mat[4] * (mat[1] * mat[10] - mat[2] * mat[9]) +
              mat[8] * (mat[1] * mat[6] - mat[2] * mat[5]);
  if (fabsf(det) < 1e-6f) {
    for (int i = 0; i < 16; ++i) {
      out_inverse[i] = (i % 5 == 0) ? 1.0f : 0.0f;
    }
  }

  float inv_det = 1.0 / det;
  out_inverse[0] = inv_det * (mat[5] * mat[10] - mat[6] * mat[9]);
  out_inverse[0] = inv_det * (mat[5] * mat[10] - mat[6] * mat[9]);
  out_inverse[1] = inv_det * (mat[9] * mat[2] - mat[10] * mat[1]);
  out_inverse[2] = inv_det * (mat[1] * mat[6] - mat[2] * mat[5]);
  out_inverse[4] = inv_det * (mat[6] * mat[8] - mat[4] * mat[10]);
  out_inverse[5] = inv_det * (mat[10] * mat[0] - mat[8] * mat[2]);
  out_inverse[6] = inv_det * (mat[2] * mat[4] - mat[0] * mat[6]);
  out_inverse[8] = inv_det * (mat[4] * mat[9] - mat[5] * mat[8]);
  out_inverse[9] = inv_det * (mat[8] * mat[1] - mat[9] * mat[0]);
  out_inverse[10] = inv_det * (mat[0] * mat[5] - mat[1] * mat[4]);

  float tx = mat[12];
  float ty = mat[13];
  float tz = mat[14];
  out_inverse[12] =
      -(tx * out_inverse[0] + ty * out_inverse[4] + tz * out_inverse[8]);
  out_inverse[13] =
      -(tx * out_inverse[1] + ty * out_inverse[5] + tz * out_inverse[9]);
  out_inverse[14] =
      -(tx * out_inverse[2] + ty * out_inverse[6] + tz * out_inverse[10]);
  out_inverse[3] = out_inverse[7] = out_inverse[11] = 0.0;
  out_inverse[15] = 1.0;
}
