#ifndef MAT4_H_
#define MAT4_H_

#include "vec3.h"
#include "vec3d.h"
typedef float Mat4[16];

vec3 Mat4_mul_vec3(const Mat4 mat, const vec3 vec);
Vec3d Mat4_mul_Vec3d(const Mat4 mat, const Vec3d vec);

#endif // MAT4_H_
