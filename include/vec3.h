#ifndef VEC3_H_
#define VEC3_H_

#include <stdbool.h>

// NOTE: Adding padding, so that this structure
// takes up exactly 16 bytes. It's necessary as OpenGL
// requires that all array elements in SSBOs are multiples of 16 bytes.
typedef struct {
  float x, y, z, _;
} vec3;

/* float3 read_float3_from_line(char *line); */
vec3 min(const vec3 a, const vec3 b);
vec3 max(const vec3 a, const vec3 b);

vec3 vec3_add(vec3 a, vec3 b);
vec3 vec3_sub(vec3 a, vec3 b);
vec3 vec3_mult(vec3 v, float c);
vec3 vec3_norm(vec3 v);
vec3 vec3_cross(vec3 a, vec3 b);

float vec3_mag(vec3 v);

// 0 - x, 1 - y, 2 - z
static inline float vec3_get_by_axis(const vec3 *v, int axis) {
  return ((float *)v)[axis];
}

void vec3_copy(const vec3 *src, vec3 *dst);
void vec3_swap(vec3 *a, vec3 *b);
/* void vec3_inc(vec3 *a, const vec3 *b); */
void vec3_from_float3(vec3 *dst, const float src[3]);
bool is_vec3_zero(vec3 v);

#endif // VEC3_H_
