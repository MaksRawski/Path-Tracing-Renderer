#ifndef BVH_H_
#define BVH_H_

#include <assert.h>
#include <stdint.h>

#include "scene/triangle.h"
#include "vec3.h"

typedef uint32_t BVHTriCount;
typedef uint32_t BVHNodeCount;

// if indexes_count > 0, then node is a leaf and contains a list of primitives
// if indexes_count == 0, then node is a branch and contains an index to the
// left child
typedef struct {
  vec3 bound_min, bound_max;
  BVHTriCount first, count;
  uint64_t _;
} BVHNode;
static_assert(sizeof(BVHNode) % 16 == 0,
              "BVHNode's size should be a multiple of 16");

typedef void FindBestSplitFn(const BVHNode *node, const Triangle *triangles,
                             const vec3 *centroids, int *best_axis,
                             float *best_split_pos);

void BVH_build(BVHNode *nodes, BVHNodeCount *nodes_offset,
               BVHTriCount *swaps_lut, Triangle triangles[],
               BVHTriCount tri_offset, BVHTriCount tri_count,
               FindBestSplitFn find_best_split_fn);

#define SWAP(_a, _b, _type)                                                    \
  do {                                                                         \
    _type tmp = _a;                                                            \
    _a = _b;                                                                   \
    _b = tmp;                                                                  \
  } while (0);

/* #include "stdlib.h" */
/* #define BVH_apply_swaps_lut(_lut, _objects, _object_type, _count)              \ */
/*   do {                                                                         \ */
/*     _object_type *_object_type##_copy = malloc(_count * sizeof(_object_type)); \ */
/*     for (unsigned long _i = 0; _i < _count; ++_i)                              \ */
/*       _object_type##_copy[_i] = _objects[_i];                                  \ */
/*     for (unsigned long _i = 0; _i < _count; ++_i) {                            \ */
/*       unsigned long swap_idx = _lut[_i];                                       \ */
/*       _objects[_i] = _object_type##_copy[swap_idx];                            \ */
/*     }                                                                          \ */
/*     free(_object_type##_copy);                                                 \ */
/*   } while (0); */

#endif // BVH_H_
