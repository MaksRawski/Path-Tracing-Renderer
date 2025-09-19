#include "vec3.h"
#include <math.h>

// element-wise mium
vec3 min(const vec3 a, const vec3 b) {
  vec3 res = {.x = a.x <= b.x ? a.x : b.x,
              .y = a.y <= b.y ? a.y : b.y,
              .z = a.z <= b.z ? a.z : b.z};
  return res;
}

vec3 max(const vec3 a, const vec3 b) {
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

void vec3_copy(const vec3 *src, vec3 *dst) {
  dst->x = src->x;
  dst->y = src->y;
  dst->z = src->z;
}

bool is_vec3_zero(vec3 v) { return v.x == 0 && v.y == 0 && v.z == 0; }

void vec3_from_float3(vec3 *dst, const float src[3]) {
  dst->x = src[0];
  dst->y = src[1];
  dst->z = src[2];
}

float vec3_mag(vec3 v) {
  return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

vec3 vec3_norm(vec3 v) { return vec3_mult(v, 1.0 / vec3_mag(v)); }

vec3 vec3_cross(vec3 a, vec3 b) {
  vec3 r = {0};
  r.x = a.y * b.z - a.z * b.y;
  r.y = a.x * b.z - a.z * b.x;
  r.z = a.x * b.y - a.y * b.x;
  return r;
}
