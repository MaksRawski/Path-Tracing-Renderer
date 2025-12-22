#ifndef BVH_H_
#define BVH_H_

#include <assert.h>

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
static_assert(sizeof(BVHnode) % 16 == 0,
              "BVHNode's size should be a multiple of 16");

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
    object_type object_type##_copy[count];                                     \
    for (unsigned long i = 0; i < count; ++i)                                  \
      object_type##_copy[i] = objects[i];                                      \
    for (unsigned long i = 0; i < count; ++i) {                                \
      unsigned long swap_idx = lut[i];                                         \
      objects[i] = object_type##_copy[swap_idx];                               \
    }                                                                          \
  } while (0);

#endif // BVH_H_
