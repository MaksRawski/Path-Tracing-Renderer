#include "scene/bvh.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

void calculate_centroids(Triangle *tri, int tri_count, vec3 centroids[]);
void set_node_bounds(BVHNode *node, const Triangle tris[]);
void subdivide(BVHNode nodes[], int node_idx, Triangle tri[], vec3 centroids[],
               BVHNodeCount *created_nodes, BVHTriCount swaps_lut[]);

void tri_swap(Triangle *a, Triangle *b);
BVHTriCount split_group(Triangle *tris, vec3 *centroids, BVHTriCount first,
                        BVHTriCount count, unsigned int axis, float split_pos,
                        BVHTriCount swaps_lut[]);

// NOTE: nodes should be zero allocated for 2 * t_count * sizeof(BVHNode) bytes
// NOTE: swaps_lut should be allocated for t_count * sizeof(BVHTriCount) bytes
// NOTE: nodes_offset afterwards will contain the index after the last node
void BVH_build(BVHNode *nodes, BVHNodeCount *nodes_offset,
               BVHTriCount *swaps_lut, Triangle triangles[],
               BVHTriCount tri_offset, BVHTriCount t_count) {
  for (BVHTriCount i = 0; i < t_count; ++i)
    swaps_lut[i] = tri_offset + i;

  vec3 *centroids = malloc(t_count * sizeof(vec3));
  calculate_centroids(triangles + tri_offset, t_count, centroids);

  nodes[*nodes_offset].first = tri_offset;
  nodes[*nodes_offset].count = t_count;
  subdivide(nodes, *nodes_offset, triangles, centroids, nodes_offset,
            swaps_lut);
  ++(*nodes_offset);

  free(centroids);
}

void BVH_delete(BVH *self) {
  free(self->nodes);
  self = NULL;
}

// recursively subdivide a node until there are 2 primitives left
void subdivide(BVHNode nodes[], int node_idx, Triangle tris[], vec3 centroids[],
               BVHNodeCount *nodes_offset, BVHTriCount swaps_lut[]) {
  BVHNode *node = nodes + node_idx;
  // 0. first set the bounds, only a leaf node can be subdivided!
  set_node_bounds(node, tris);

  if (node->count <= 8)
    return;

  // 1. determine axis and position of a split
  // (right now choosing the longest axis and splitting in half)
  vec3 extent = vec3_sub(node->bound_max, node->bound_min);
  // find out longest axis
  int axis = 0;
  if (extent.y > extent.x)
    axis = 1;
  if (extent.z > extent.y && extent.z > extent.x)
    axis = 2;

  float split_pos = vec3_get_by_axis(&extent, axis) * 0.5f +
                    vec3_get_by_axis(&node->bound_min, axis);

  // 2.
  BVHTriCount split_index = split_group(
      tris, centroids, node->first, node->count, axis, split_pos, swaps_lut);

  // 3. create child nodes for the splits
  // if the split turned out to leave all elements on one side
  // then we leave that node as it was
  if (split_index == node->first || split_index == (node->first + node->count))
    return;

  int left_node_idx = ++(*nodes_offset);
  int right_node_idx = ++(*nodes_offset);
  BVHNode *left_node = &nodes[left_node_idx];
  BVHNode *right_node = &nodes[right_node_idx];
  left_node->first = node->first;
  left_node->count = split_index - node->first;
  right_node->first = split_index;
  right_node->count = node->count - left_node->count;

  // denote that this node is a parent
  node->count = 0;
  node->first = left_node_idx;

  subdivide(nodes, left_node_idx, tris, centroids, nodes_offset, swaps_lut);
  subdivide(nodes, right_node_idx, tris, centroids, nodes_offset, swaps_lut);
}

// centroids should already have an allocated memory for tri_count of vec3
void calculate_centroids(Triangle *tri, int tri_count, vec3 *centroids) {
  for (int i = 0; i < tri_count; ++i) {
    centroids[i] =
        vec3_mult(vec3_add(vec3_add(tri[i].a, tri[i].b), tri[i].c), 0.333f);
  }
}

// sets node bounds on a leaf node
void set_node_bounds(BVHNode *node, const Triangle *tris) {
  int last_tri = node->first + node->count;
  node->bound_min.x = INFINITY;
  node->bound_min.y = INFINITY;
  node->bound_min.z = INFINITY;

  node->bound_max.x = -INFINITY;
  node->bound_max.y = -INFINITY;
  node->bound_max.z = -INFINITY;

  for (int t = node->first; t < last_tri; ++t) {
    node->bound_min = vec3_min(node->bound_min, tris[t].a);
    node->bound_min = vec3_min(node->bound_min, tris[t].b);
    node->bound_min = vec3_min(node->bound_min, tris[t].c);
    node->bound_max = vec3_max(node->bound_max, tris[t].a);
    node->bound_max = vec3_max(node->bound_max, tris[t].b);
    node->bound_max = vec3_max(node->bound_max, tris[t].c);
  }
}

void tri_swap(Triangle *a, Triangle *b) {
  Triangle t = *a;
  *a = *b;
  *b = t;
}

// Swaps positions of triangles (and centroids) so that all to the "left" of
// split_pos are before or at split index and all to the "right" of the
// split_pos are after index. Returns the split index.
BVHTriCount split_group(Triangle tris[], vec3 centroids[], BVHTriCount first,
                        BVHTriCount count, unsigned int axis, float split_pos,
                        BVHTriCount swaps_lut[]) {
  int i = first;
  int j = i + count - 1;
  while (i <= j) {
    if (vec3_get_by_axis(&centroids[i], axis) < split_pos)
      ++i;
    else {
      SWAP(tris[i], tris[j], Triangle);
      SWAP(centroids[i - first], centroids[j - first], vec3);
      SWAP(swaps_lut[i - first], swaps_lut[j - first], BVHTriCount)

      --j;
    }
  }
  return i;
}
