#include "arena.h"
#include "asserts.h"
#include "scene.h"
#include "scene/mesh.h"
#include <alloca.h>
#include <math.h>

// NOTE: only sets bounds_min and bounds_max on the returned node
static BVHNode transform_bounds(vec3 min, vec3 max, Mat4 transform) {
  vec3 corners[8] = {
      vec3_new(min.x, min.y, min.z), vec3_new(min.x, min.y, max.z),
      vec3_new(min.x, max.y, min.z), vec3_new(min.x, max.y, max.z),
      vec3_new(max.x, min.y, min.z), vec3_new(max.x, min.y, max.z),
      vec3_new(max.x, max.y, min.z), vec3_new(max.x, max.y, max.z),
  };

  vec3 new_min = vec3_new(INFINITY, INFINITY, INFINITY);
  vec3 new_max = vec3_new(-INFINITY, -INFINITY, -INFINITY);
  for (unsigned int c = 0; c < 8; ++c) {
    vec3 transformed = Mat4_mul_vec3(transform, corners[c]);
    new_min = vec3_min(new_min, transformed);
    new_max = vec3_max(new_max, transformed);
  }

  return (BVHNode){.bound_min = new_min, .bound_max = new_max};
}

// NOTE: also initializes the unmatched_nodes array
static void create_leaf_nodes(Scene *scene, unsigned int unmatched_nodes[]) {
  // expecting tlas_nodes to not yet be initialized
  ASSERTQ_EQ(scene->tlas_nodes_count, 0);
  // reserving space for the root node
  scene->tlas_nodes_count = 1;

  for (unsigned int i = 0; i < scene->mesh_instances_count; ++i) {
    unmatched_nodes[i] = scene->tlas_nodes_count;

    MeshInstance *mesh_instance = &scene->mesh_instances[i];
    Mesh *mesh = &scene->meshes[mesh_instance->mesh_index];
    BVHNode transformed_mi_bounds = transform_bounds(
        mesh->aabbMin, mesh->aabbMax, mesh_instance->transform);
    scene->tlas_nodes[scene->tlas_nodes_count++] =
        (TLASNode){.aabbMin = transformed_mi_bounds.bound_min,
                   .aabbMax = transformed_mi_bounds.bound_max,
                   .left = 0,
                   .right = 0,
                   .mesh_instance = i};
  }
}

// NOTE: A is supposed to be an index to unmatched_nodes which then in turn
// contains the index to nodes
static int find_best_match(const TLASNode *nodes,
                           const unsigned int unmatched_nodes[],
                           unsigned int unmatched_nodes_count,
                           unsigned int un_A_idx) {
  float smallest_sa = INFINITY;
  int best_B = -1;
  for (unsigned int B = 0; B < unmatched_nodes_count; ++B) {
    if (B == un_A_idx)
      continue;
    vec3 new_bound_min = vec3_min(nodes[unmatched_nodes[un_A_idx]].aabbMin,
                                  nodes[unmatched_nodes[B]].aabbMin);
    vec3 new_bound_max = vec3_max(nodes[unmatched_nodes[un_A_idx]].aabbMax,
                                  nodes[unmatched_nodes[B]].aabbMax);
    vec3 extent = vec3_sub(new_bound_max, new_bound_min);
    // NOTE: this is just the half of the surface area as it's just used for
    // comparison in this function
    float sa = extent.x * extent.y + extent.y * extent.z + extent.z * extent.x;
    if (sa < smallest_sa) {
      smallest_sa = sa;
      best_B = B;
    }
  }

  return best_B;
}

// heavily inspired by:
// https://jacco.ompf2.com/2022/05/13/how-to-build-a-bvh-part-6-all-together-now/
// nodeIdx - unmatched_nodes
// nodeIndices - unmatched_nodes_count
// nodesUsed - scene->tlas_nodes_count
void Scene_build_tlas(Scene *scene, Arena *arena) {
  if (Scene_is_empty(scene))
    return;

  ArenaSnapshot as = Arena_snapshot(arena);
  unsigned int *unmatched_nodes =
      Arena_alloc(arena, scene->mesh_instances_count * sizeof(unsigned int));

  create_leaf_nodes(scene, unmatched_nodes);
  unsigned int unmatched_nodes_count = scene->mesh_instances_count;

  // prefix 'un' used for indices of the unmatched_nodes array
  int un_A_idx = 0;
  int un_B_idx = find_best_match(scene->tlas_nodes, unmatched_nodes,
                                 unmatched_nodes_count, un_A_idx);

  while (unmatched_nodes_count > 1) {
    int un_C_idx = find_best_match(scene->tlas_nodes, unmatched_nodes,
                                   unmatched_nodes_count, un_B_idx);
    if (un_A_idx == un_C_idx) {
      int A_idx = unmatched_nodes[un_A_idx];
      int B_idx = unmatched_nodes[un_B_idx];
      TLASNode *A = &scene->tlas_nodes[A_idx];
      TLASNode *B = &scene->tlas_nodes[B_idx];
      TLASNode *new_node = &scene->tlas_nodes[scene->tlas_nodes_count];

      new_node->left = A_idx;
      new_node->right = B_idx;
      new_node->aabbMin = vec3_min(A->aabbMin, B->aabbMin);
      new_node->aabbMax = vec3_max(A->aabbMax, B->aabbMax);

      unmatched_nodes[un_A_idx] = scene->tlas_nodes_count++;
      unmatched_nodes[un_B_idx] = unmatched_nodes[unmatched_nodes_count - 1];
      un_B_idx = find_best_match(scene->tlas_nodes, unmatched_nodes,
                                 --unmatched_nodes_count, un_A_idx);
    } else {
      un_A_idx = un_B_idx;
      un_B_idx = un_C_idx;
    }
  }
  scene->tlas_nodes[0] = scene->tlas_nodes[unmatched_nodes[un_A_idx]];
  Arena_rewind(as);
}
