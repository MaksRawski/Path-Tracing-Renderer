#ifndef BVH_H_
#define BVH_H_

#include "scene/triangle.h" 
#include "vec3.h"

// if indexes_count > 0, then node is a leaf and contains a list of primitives
// if indexes_count == 0, then node is a branch and contains an index to the
// left child
typedef struct {
  vec3 bound_min, bound_max;
  int first, count;
  long _;
} BVHnode;

// node index 0 must be root
typedef struct {
  BVHnode *nodes;
  int nodes_count;
} BVH;

typedef struct {
  BVH bvh;
  // LUT which describes how triangles have been swapped around
  int *swaps_lut;
} BVHresult;

BVHresult BVH_build(Triangle *tri, int tri_count);
void BVH_delete(BVH *self);

#endif // BVH_H_
