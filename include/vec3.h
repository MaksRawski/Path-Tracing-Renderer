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

/* float3 read_float3_from_line(char *line); */
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
/* void vec3_inc(vec3 *a, const vec3 *b); */
bool vec3_is_zero(vec3 v);
bool vec3_eq(vec3 a, vec3 b);

typedef struct {
  char s[32];
} Vec3Str;

// returns a string representation of a vector
Vec3Str vec3_str(vec3 v);

#endif // VEC3_H_
