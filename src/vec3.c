#include "vec3.h"
#include "epsilon.h"
#include <math.h>
#include <stdio.h>

vec3 vec3_new(float x, float y, float z) { return (vec3){x, y, z, 0}; }
// creates a vec3 from 3 consecutive floats
vec3 vec3_from_float3(const float *const v) {
  return (vec3){v[0], v[1], v[2], 0};
}

// element-wise min
vec3 vec3_min(vec3 a, vec3 b) {
  vec3 res = {.x = a.x <= b.x ? a.x : b.x,
              .y = a.y <= b.y ? a.y : b.y,
              .z = a.z <= b.z ? a.z : b.z};
  return res;
}

// element-wise max
vec3 vec3_max(vec3 a, vec3 b) {
  vec3 res = {.x = a.x >= b.x ? a.x : b.x,
              .y = a.y >= b.y ? a.y : b.y,
              .z = a.z >= b.z ? a.z : b.z};
  return res;
}

vec3 vec3_add(vec3 a, vec3 b) {
  vec3 res;
  res.x = a.x + b.x;
  res.y = a.y + b.y;
  res.z = a.z + b.z;
  return res;
}

vec3 vec3_sub(vec3 a, vec3 b) {
  vec3 res;
  res.x = a.x - b.x;
  res.y = a.y - b.y;
  res.z = a.z - b.z;
  return res;
}

vec3 vec3_mult(vec3 v, float c) {
  vec3 res;
  res.x = v.x * c;
  res.y = v.y * c;
  res.z = v.z * c;
  return res;
}

void vec3_swap(vec3 *a, vec3 *b) {
  vec3 t = *a;
  *a = *b;
  *b = t;
}
void vec3_copy_from_float3(vec3 *const dst, const float *const src) {
  dst->x = src[0];
  dst->y = src[1];
  dst->z = src[2];
}

bool vec3_is_zero(vec3 v) {
  return is_zero(v.x) && is_zero(v.y) && is_zero(v.z);
}
bool vec3_eq(vec3 a, vec3 b) {
  return vec3_is_zero(vec3_sub(a, b));
}

float vec3_mag(vec3 v) { return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z); }
vec3 vec3_norm(vec3 v) { return vec3_mult(v, 1.0 / vec3_mag(v)); }

vec3 vec3_cross(vec3 a, vec3 b) {
  vec3 r = {0};
  r.x = a.y * b.z - a.z * b.y;
  r.y = a.x * b.z - a.z * b.x;
  r.z = a.x * b.y - a.y * b.x;
  return r;
}

Vec3Str vec3_str(vec3 v) {
  Vec3Str res = {0};
  sprintf(res.s, "%+.3f %+.3f %+.3f", v.x, v.y, v.z);
  return res;
}
