#include "mat4.h"

// assumes a column-major matrix
vec3 mat4_mul_vec3(Mat4 mat, vec3 vec) {
  vec3 v = {0};
  v.x = mat[0] * vec.x + mat[4] * vec.y + mat[8] * vec.z + mat[12] * 1;
  v.y = mat[1] * vec.x + mat[5] * vec.y + mat[9] * vec.z + mat[13] * 1;
  v.z = mat[2] * vec.x + mat[6] * vec.y + mat[10] * vec.z + mat[14] * 1;

  return v;
}
