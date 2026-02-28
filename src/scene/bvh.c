#include "scene/bvh.h"
#include "arena.h"
#include "scene/aabb.h"
#include "vec3.h"
#include <math.h>

static void calculate_centroids(Triangle *tri, int tri_count, vec3 centroids[]);
static void set_node_bounds(BVHnode *node, const Triangle tris[]);
static void subdivide(BVHnode nodes[], int node_idx, Triangle tri[],
                      vec3 centroids[], BVHNodeCount *created_nodes,
                      BVHTriCount swaps_lut[],
                      FindBestSplitFn find_best_split_fn);
static BVHTriCount split_group(Triangle *tris, vec3 *centroids,
                               BVHTriCount first, BVHTriCount count,
                               unsigned int axis, float split_pos,
                               BVHTriCount swaps_lut[]);

// NOTE: nodes should be zero allocated for 2 * t_count * sizeof(BVHNode) bytes
// NOTE: swaps_lut should be allocated for t_count * sizeof(BVHTriCount) bytes
// NOTE: nodes_offset afterwards will contain the index after the last node
void BVH_build(BVHnode *nodes, BVHNodeCount *nodes_offset,
               BVHSwapsLUTElement *swaps_lut, Triangle triangles[],
               BVHTriCount tri_offset, BVHTriCount t_count,
               FindBestSplitFn find_best_split_fn, Arena *arena) {
  for (BVHTriCount i = 0; i < t_count; ++i)
    swaps_lut[i] = tri_offset + i;

  ArenaSnapshot as = Arena_snapshot(arena);
  vec3 *centroids = Arena_alloc(arena, t_count * sizeof(vec3));
  calculate_centroids(triangles + tri_offset, t_count, centroids);

  nodes[*nodes_offset].first = tri_offset;
  nodes[*nodes_offset].count = t_count;
  subdivide(nodes, *nodes_offset, triangles, centroids, nodes_offset, swaps_lut,
            find_best_split_fn);
  ++(*nodes_offset);

  Arena_rewind(as);
}

// recursively subdivide a node until there are 2 primitives left
void subdivide(BVHnode nodes[], int node_idx, Triangle tris[], vec3 centroids[],
               BVHNodeCount *created_nodes, BVHTriCount swaps_lut[],
               FindBestSplitFn find_best_split_fn) {
  BVHnode *node = nodes + node_idx;
  // 0. first set the bounds, only a leaf node can be subdivided!
  set_node_bounds(node, tris);

  if (node->count <= 8)
    return;

  // 1. determine axis and position of a split
  int axis = -1;
  float split_pos = -INFINITY;
  find_best_split_fn(node, tris, centroids, &axis, &split_pos);
  if (axis == -1 || split_pos == -INFINITY)
    return;

  // 2.
  BVHTriCount split_index = split_group(
      tris, centroids, node->first, node->count, axis, split_pos, swaps_lut);

  // 3. create child nodes for the splits
  // if the split turned out to leave all elements on one side
  // then we leave that node as it was
  if (split_index == node->first || split_index == (node->first + node->count))
    return;

  int left_node_idx = ++(*created_nodes);
  int right_node_idx = ++(*created_nodes);
  BVHnode *left_node = &nodes[left_node_idx];
  BVHnode *right_node = &nodes[right_node_idx];
  left_node->first = node->first;
  left_node->count = split_index - node->first;
  right_node->first = split_index;
  right_node->count = node->count - left_node->count;

  // denote that this node is a parent
  node->count = 0;
  node->first = left_node_idx;

  subdivide(nodes, left_node_idx, tris, centroids, created_nodes, swaps_lut,
            find_best_split_fn);
  subdivide(nodes, right_node_idx, tris, centroids, created_nodes, swaps_lut,
            find_best_split_fn);
}

// centroids should already have an allocated memory for tri_count of vec3
void calculate_centroids(Triangle *tri, int tri_count, vec3 *centroids) {
  for (int i = 0; i < tri_count; ++i) {
    centroids[i] =
        vec3_mult(vec3_add(vec3_add(tri[i].a, tri[i].b), tri[i].c), 0.333f);
  }
}

// sets node bounds on a leaf node
void set_node_bounds(BVHnode *node, const Triangle *tris) {
  AABB aabb = AABB_new();
  int last_tri = node->first + node->count;
  for (int t = node->first; t < last_tri; ++t) {
    AABB_grow_tri(&aabb, &tris[t]);
  }
  node->bound_min = aabb.min;
  node->bound_max = aabb.max;
}

// Swaps positions of triangles (and centroids) so that all to the "left" of
// split_pos are before or at split index and all to the "right" of the
// split_pos are after index. Returns the split index.
static BVHTriCount split_group(Triangle tris[], vec3 centroids[],
                               BVHTriCount first, BVHTriCount count,
                               unsigned int axis, float split_pos,
                               BVHTriCount swaps_lut[]) {
  int i = first;
  int j = i + count - 1;
  while (i <= j) {
    if (vec3_get_by_axis(&centroids[i], axis) < split_pos)
      ++i;
    else {
      SWAP(tris[i], tris[j], Triangle);
      SWAP(centroids[i], centroids[j], vec3);
      SWAP(swaps_lut[i], swaps_lut[j], BVHTriCount)

      --j;
    }
  }
  return i;
}
