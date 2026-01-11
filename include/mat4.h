#ifndef MAT4_H_
#define MAT4_H_

#include "vec3.h"

typedef float Mat4[16];

vec3 Mat4_mul_vec3(const Mat4 mat, const vec3 vec);

void Mat4_trs_inverse(const Mat4 mat, Mat4 out_inverse);

#endif // MAT4_H_
