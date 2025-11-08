#include "vec3d.h"
#include "epsilon.h"
#include <math.h>
#include <stdio.h>

Vec3d Vec3d_new(double x, double y, double z) { return (Vec3d){x, y, z}; }
// creates a Vec3d from 3 consecutive doubles
Vec3d Vec3d_from_double3(const double *const v) {
  return Vec3d_new(v[0], v[1], v[2]);
}

vec3 Vec3d_to_vec3(Vec3d v) { return vec3_new(v.x, v.y, v.z); }
Vec3d Vec3d_from_vec3(vec3 v) { return Vec3d_new(v.x, v.y, v.z); }

// element-wise min
Vec3d Vec3d_min(Vec3d a, Vec3d b) {
  Vec3d res = {.x = a.x <= b.x ? a.x : b.x,
               .y = a.y <= b.y ? a.y : b.y,
               .z = a.z <= b.z ? a.z : b.z};
  return res;
}

// element-wise max
Vec3d Vec3d_max(Vec3d a, Vec3d b) {
  Vec3d res = {.x = a.x >= b.x ? a.x : b.x,
               .y = a.y >= b.y ? a.y : b.y,
               .z = a.z >= b.z ? a.z : b.z};
  return res;
}

Vec3d Vec3d_add(Vec3d a, Vec3d b) {
  Vec3d res;
  res.x = a.x + b.x;
  res.y = a.y + b.y;
  res.z = a.z + b.z;
  return res;
}

Vec3d Vec3d_sub(Vec3d a, Vec3d b) {
  Vec3d res;
  res.x = a.x - b.x;
  res.y = a.y - b.y;
  res.z = a.z - b.z;
  return res;
}

Vec3d Vec3d_mult(Vec3d v, double c) {
  Vec3d res;
  res.x = v.x * c;
  res.y = v.y * c;
  res.z = v.z * c;
  return res;
}

void Vec3d_swap(Vec3d *a, Vec3d *b) {
  Vec3d t = *a;
  *a = *b;
  *b = t;
}
void Vec3d_copy_from_double3(Vec3d *const dst, const double *const src) {
  dst->x = src[0];
  dst->y = src[1];
  dst->z = src[2];
}

bool Vec3d_is_zero(Vec3d v) {
  return is_zero(v.x) && is_zero(v.y) && is_zero(v.z);
}
bool Vec3d_eq(Vec3d a, Vec3d b) { return Vec3d_is_zero(Vec3d_sub(a, b)); }

double Vec3d_mag(Vec3d v) { return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z); }
Vec3d Vec3d_norm(Vec3d v) { return Vec3d_mult(v, 1.0 / Vec3d_mag(v)); }

Vec3d Vec3d_cross(Vec3d a, Vec3d b) {
  Vec3d r = {0};
  r.x = a.y * b.z - a.z * b.y;
  r.y = a.x * b.z - a.z * b.x;
  r.z = a.x * b.y - a.y * b.x;
  return r;
}

Vec3dStr Vec3d_str(Vec3d v) {
  Vec3dStr res = {0};
  sprintf(res.s, "%+.3f %+.3f %+.3f", v.x, v.y, v.z);
  return res;
}
