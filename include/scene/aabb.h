#ifndef AABB_H_
#define AABB_H_

#include "scene/triangle.h"
#include "vec3.h"

typedef struct {
  vec3 min, max;
} AABB;

AABB AABB_new(void);
AABB AABB_from(vec3 min, vec3 max);

vec3 AABB_extent(const AABB *self);
float AABB_area(const AABB *self);

void AABB_grow(AABB *self, vec3 p);
void AABB_grow_tri(AABB *self, const Triangle *t);

#endif // AABB_H_
