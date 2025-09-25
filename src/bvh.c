#include "bvh.h"
#include <math.h>
#include <stdlib.h>

BVHresult build_bvh(Triangle *tri, int tri_count) {
  BVHresult res = {0};
  res.bvh.nodes = calloc(tri_count * 2 - 1, sizeof(BVHnode));
  res.swaps_lut = malloc(tri_count * sizeof(int));
  for (int i = 0; i < tri_count; ++i)
    res.swaps_lut[i] = i;

  vec3 *centroids = malloc(tri_count * sizeof(vec3));
  calculate_centroids(tri, tri_count, centroids);

  res.bvh.nodes_count = 1;
  res.bvh.nodes[0].count = tri_count;
  subdivide(res.bvh.nodes, 0, tri, centroids, &res.bvh.nodes_count,
            res.swaps_lut);

  free(centroids);
  return res;
}

// recursively subdivide a node until there are 2 primitives left
void subdivide(BVHnode *nodes, int node_idx, Triangle *tris, vec3 *centroids,
               int *created_nodes, int swaps_lut[]) {
  BVHnode *node = nodes + node_idx;
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
  int split_index = split_group(tris, centroids, node->first, node->count, axis,
                                split_pos, swaps_lut);

  // 3. create child nodes for the splits
  // if the split turned out to leave all elements on one side
  // then we leave that node as it was
  if (split_index == node->first ||
      split_index == (int)(node->first + node->count))
    return;

  int left_node_idx = (*created_nodes)++;
  int right_node_idx = (*created_nodes)++;
  BVHnode *left_node = &nodes[left_node_idx];
  BVHnode *right_node = &nodes[right_node_idx];
  left_node->first = node->first;
  left_node->count = split_index - node->first;
  right_node->first = split_index;
  right_node->count = node->count - left_node->count;

  // denote that this node is a parent
  node->count = 0;
  node->first = left_node_idx;

  subdivide(nodes, left_node_idx, tris, centroids, created_nodes, swaps_lut);
  subdivide(nodes, right_node_idx, tris, centroids, created_nodes, swaps_lut);
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
int split_group(Triangle *tris, vec3 *centroids, int first, int count, int axis,
                float split_pos, int swaps_lut[]) {
  int i = first;
  int j = i + count - 1;
  while (i <= j) {
    if (vec3_get_by_axis(&centroids[i], axis) < split_pos)
      ++i;
    else {
      tri_swap(tris + i, tris + j);
      vec3_swap(centroids + i, centroids + j);

      int tmp = swaps_lut[i];
      swaps_lut[i] = swaps_lut[j];
      swaps_lut[j] = tmp;

      --j;
    }
  }
  return i;
}
