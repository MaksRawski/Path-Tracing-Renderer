#include "scene/bvh/strategies.h"
#include "asserts.h"
#include "scene/aabb.h"
#include <math.h>

void FindBestSplitFn_longest_mid(const BVHNode *node, const Triangle *triangles,
                                 const vec3 *centroids, int *best_axis,
                                 float *best_split_pos) {
  UNUSED(triangles, centroids);
  vec3 extent = vec3_sub(node->bound_max, node->bound_min);
  // find out longest axis
  *best_axis = 0;
  if (extent.y > extent.x)
    *best_axis = 1;
  if (extent.z > extent.y && extent.z > extent.x)
    *best_axis = 2;

  *best_split_pos = vec3_get_by_axis(&extent, *best_axis) * 0.5f +
                    vec3_get_by_axis(&node->bound_min, *best_axis);
}

static float SAH_cost(const BVHNode *node, const Triangle *triangles,
                      const vec3 *centroids, int axis, float split_pos) {
  AABB left = AABB_new(), right = AABB_new();
  unsigned int left_count = 0, right_count = 0;
  for (unsigned int i = 0; i < node->count; ++i) {
    const Triangle *tri = &triangles[node->first + i];
    const vec3 *centroid = &centroids[node->first + i];
    if (vec3_get_by_axis(centroid, axis) < split_pos) {
      ++left_count;
      AABB_grow_tri(&left, tri);
    } else {
      ++right_count;
      AABB_grow_tri(&right, tri);
    }
  }
  float cost = left_count * AABB_area(&left) + right_count * AABB_area(&right);
  return cost > 0 ? cost : INFINITY;
}

// https://jacco.ompf2.com/2022/04/18/how-to-build-a-bvh-part-2-faster-rays/
void FindBestSplitFn_SAH(const BVHNode *node, const Triangle *triangles,
                         const vec3 *centroids, int *best_axis,
                         float *best_split_pos) {
  AABB parent_aabb = AABB_from(node->bound_min, node->bound_max);
  float parent_cost = node->count * AABB_area(&parent_aabb);
  float best_cost = parent_cost;
  for (unsigned int axis = 0; axis < 3; ++axis) {
    for (unsigned int i = 0; i < node->count; ++i) {
      const vec3 *centroid = &centroids[node->first + i];
      float split_pos = vec3_get_by_axis(centroid, axis);
      float cost = SAH_cost(node, triangles, centroids, axis, split_pos);
      if (cost < best_cost) {
        best_cost = cost;
        *best_axis = axis;
        *best_split_pos = split_pos;
      }
    }
  }
}
