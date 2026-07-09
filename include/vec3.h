#ifndef VEC3_H_
#define VEC3_H_

#include <stdbool.h>

// NOTE: Adding padding, so that this structure
// takes up exactly 16 bytes. It's necessary as OpenGL
// requires that all array elements in SSBOs are multiples of 16 bytes.
typedef struct {
  float x, y, z, _;
} vec3;

vec3 vec3_new(float x, float y, float z);
vec3 vec3_from_float3(const float *const v);

vec3 vec3_min(vec3 a, vec3 b);
vec3 vec3_max(vec3 a, vec3 b);

vec3 vec3_add(vec3 a, vec3 b);
vec3 vec3_sub(vec3 a, vec3 b);
vec3 vec3_mult(vec3 v, float c);
vec3 vec3_norm(vec3 v);
vec3 vec3_cross(vec3 a, vec3 b);

float vec3_mag(vec3 v);

// 0 - x, 1 - y, 2 - z
static inline float vec3_get_by_axis(const vec3 *const v, int axis) {
  return ((const float *)v)[axis];
}

void vec3_copy_from_float3(vec3 *const dst, const float *const src);
void vec3_swap(vec3 *a, vec3 *b);
bool vec3_eq(vec3 a, vec3 b, float epsilon);

// NOTE: vec3_str is not provided as it's up to the caller to decide what format
// they want and at that point there is no point in making an abstraction around it

#endif // VEC3_H_
