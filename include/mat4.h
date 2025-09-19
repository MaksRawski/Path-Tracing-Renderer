#ifndef MAT4_H_
#define MAT4_H_

#include "vec3.h"
typedef float Mat4[16];

vec3 mat4_mul_vec3(Mat4 mat, vec3 vec);

#endif // MAT4_H_
