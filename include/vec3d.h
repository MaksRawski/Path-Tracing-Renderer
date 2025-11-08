#ifndef VEC3D_H_
#define VEC3D_H_

#include "vec3.h"
#include <stdbool.h>

// NOTE: Adding padding, so that this structure
// takes up exactly 16 bytes. It's necessary as OpenGL
// requires that all array elements in SSBOs are multiples of 16 bytes.
typedef struct {
  double x, y, z;
} Vec3d;

Vec3d Vec3d_new(double x, double y, double z);
Vec3d Vec3d_from_double3(const double *const v);
vec3 Vec3d_to_vec3(Vec3d v);
Vec3d Vec3d_from_vec3(vec3 v);

/* double3 read_double3_from_line(char *line); */
Vec3d Vec3d_min(Vec3d a, Vec3d b);
Vec3d Vec3d_max(Vec3d a, Vec3d b);

Vec3d Vec3d_add(Vec3d a, Vec3d b);
Vec3d Vec3d_sub(Vec3d a, Vec3d b);
Vec3d Vec3d_mult(Vec3d v, double c);
Vec3d Vec3d_norm(Vec3d v);
Vec3d Vec3d_cross(Vec3d a, Vec3d b);

double Vec3d_mag(Vec3d v);

// 0 - x, 1 - y, 2 - z
static inline double Vec3d_get_by_axis(const Vec3d *const v, int axis) {
  return ((double *)v)[axis];
}

void Vec3d_copy_from_double3(Vec3d *const dst, const double *const src);
void Vec3d_swap(Vec3d *a, Vec3d *b);
bool Vec3d_is_zero(Vec3d v);
bool Vec3d_eq(Vec3d a, Vec3d b);

typedef struct {
  char s[64];
} Vec3dStr;

// returns a string representation of a vector
Vec3dStr Vec3d_str(Vec3d v);

#endif // VEC3D_H_
