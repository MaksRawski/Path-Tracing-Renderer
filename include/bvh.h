#ifndef BVH_H_
#define BVH_H_

#include "structs.h" // Triangle
#include "vec3.h"

// if indexes_count > 0, then node is a leaf and contains a list of primitives
// if indexes_count == 0, then node is a branch and contains an index to the
// left child
typedef struct {
  vec3 bound_min, bound_max;
  int first, count;
  long _; // TODO: padding to check if nodes work correctly now
} BVHnode;

// node index 0 must be root
typedef struct {
  BVHnode *nodes;
  Primitive *primitives;
  int nodes_count;
} BVH;

typedef struct {
  BVH bvh;
  // LUT which describes how triangles have been swapped around
  int *swaps_lut;
} BVHresult;

BVHresult build_bvh(Triangle *tri, int tri_count);

void calculate_centroids(Triangle *tri, int tri_count, vec3 *centroids);
void set_node_bounds(BVHnode *node, const Triangle *tris);
void subdivide(BVHnode *nodes, int node_idx, Triangle *tri, vec3 *centroids,
               int *created_nodes, int swaps_lut[]);

void tri_swap(Triangle *a, Triangle *b);
int split_group(Triangle *tris, vec3 *centroids, int first, int count, int axis,
                float split_pos, int swaps_lut[]);

#endif // BVH_H_
