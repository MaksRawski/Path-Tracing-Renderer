#ifndef BVH_H_
#define BVH_H_

#include "scene/triangle.h"
#include "vec3.h"

typedef unsigned int BVHTriCount;
typedef unsigned int BVHNodeCount;

// if indexes_count > 0, then node is a leaf and contains a list of primitives
// if indexes_count == 0, then node is a branch and contains an index to the
// left child
typedef struct {
  vec3 bound_min, bound_max;
  BVHTriCount first, count;
  long _;
} BVHnode;

// node index 0 must be root
typedef struct {
  BVHnode *nodes;
  BVHNodeCount nodes_count;
} BVH;

typedef struct {
  BVH bvh;
  // LUT which describes how triangles have been swapped around
  BVHTriCount *swaps_lut;
} BVHresult;

BVHresult BVH_build(Triangle triangles[], BVHTriCount count);
void BVH_delete(BVH *self);

#define SWAP(a, b, type)                                                       \
  do {                                                                         \
    type tmp = a;                                                              \
    a = b;                                                                     \
    b = tmp;                                                                   \
  } while (0);

#define BVH_apply_swaps_lut(lut, objects, object_type, count)                  \
  do {                                                                         \
    for (unsigned long i = 0; i < count; ++i) {                                \
      unsigned long swap_idx = lut[i];                                         \
      if (i < swap_idx) {                                                      \
        SWAP(objects[i], objects[swap_idx], object_type);                      \
      }                                                                        \
    }                                                                          \
  } while (0);

#endif // BVH_H_
