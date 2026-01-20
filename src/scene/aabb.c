#include "scene/aabb.h"
#include <math.h>

AABB AABB_new(void) {
  return (AABB){.min = vec3_new(INFINITY, INFINITY, INFINITY),
                .max = vec3_new(-INFINITY, -INFINITY, -INFINITY)};
}
AABB AABB_from(vec3 min, vec3 max) { return (AABB){.min = min, .max = max}; }

vec3 AABB_extent(const AABB *self) { return vec3_sub(self->max, self->min); }
float AABB_area(const AABB *self) {
  vec3 e = AABB_extent(self);
  return e.x * e.y + e.y * e.z + e.z * e.x;
}

void AABB_grow(AABB *self, vec3 p) {
  self->min = vec3_min(self->min, p);
  self->max = vec3_max(self->max, p);
}
void AABB_grow_tri(AABB *self, const Triangle *t) {
  AABB_grow(self, t->a);
  AABB_grow(self, t->b);
  AABB_grow(self, t->c);
}
