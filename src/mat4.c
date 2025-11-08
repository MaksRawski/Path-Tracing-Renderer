#include "mat4.h"

// assumes a column-major matrix
vec3 Mat4_mul_vec3(const Mat4 mat, const vec3 vec) {
  vec3 v = {0};
  v.x = mat[0] * vec.x + mat[4] * vec.y + mat[8] * vec.z + mat[12] * 1;
  v.y = mat[1] * vec.x + mat[5] * vec.y + mat[9] * vec.z + mat[13] * 1;
  v.z = mat[2] * vec.x + mat[6] * vec.y + mat[10] * vec.z + mat[14] * 1;

  return v;
}

Vec3d Mat4_mul_Vec3d(const Mat4 mat, const Vec3d vec) {
  Vec3d v = {0};
  v.x = mat[0] * vec.x + mat[4] * vec.y + mat[8] * vec.z + mat[12] * 1;
  v.y = mat[1] * vec.x + mat[5] * vec.y + mat[9] * vec.z + mat[13] * 1;
  v.z = mat[2] * vec.x + mat[6] * vec.y + mat[10] * vec.z + mat[14] * 1;

  return v;
}
