#include "arena.h"
#include "asserts.h"
#include "scene.h"
#include <math.h>

static void Scene_set_meshes_bounds(Scene *scene) {
  for (unsigned int mi = 0; mi < scene->mesh_instances_count; ++mi) {
    Mesh *mesh = &scene->meshes[scene->mesh_instances[mi].mesh_index];
    // skip meshes with already set bounds
    if (mesh->aabbMin.x != INFINITY)
      continue;

    for (unsigned int mpi = 0; mpi < mesh->mesh_primitive_count; ++mpi) {
      const MeshPrimitive *mesh_primitive =
          &scene->mesh_primitives[mesh->mesh_primitive_first + mpi];
      BVHNode *bvh_root = &scene->bvh_nodes[mesh_primitive->BVH_index];
      mesh->aabbMin = vec3_min(mesh->aabbMin, bvh_root->bound_min);
      mesh->aabbMax = vec3_max(mesh->aabbMax, bvh_root->bound_max);
    }
  }
}

typedef struct {
  unsigned int first, count;
} TriangleList;

typedef struct {
  TriangleList *bvh_roots;
  unsigned int *swaps_lut;
} build_bvh_tmp;

// builds BVH for each MeshPrimitive
// NOTE: assumes that each MeshPrimitive has BVH_index set to a BVHNode that
// contains `first` and `count` fields set to an appropriate list of triangles
// for that MeshPrimitive
// NOTE: changes BVH_index of each MeshPrimitive
void Scene_build_blas(Scene *scene, BVHStrategy strategy, Arena *arena) {
  if (Scene_is_empty(scene))
    return;

  build_bvh_tmp tmps = {0};
  ArenaSnapshot as = Arena_snapshot(arena);
  tmps.bvh_roots =
      Arena_alloc(arena, scene->mesh_primitives_count * sizeof(TriangleList));

  for (unsigned int mp = 0; mp < scene->mesh_primitives_count; ++mp) {
    tmps.bvh_roots[mp].first =
        scene->bvh_nodes[scene->mesh_primitives[mp].BVH_index].first;
    tmps.bvh_roots[mp].count =
        scene->bvh_nodes[scene->mesh_primitives[mp].BVH_index].count;
  }

  ASSERTQ_CUSTOM(scene->bvh_nodes_capacity >= 2 * scene->triangles_count,
                 "bvh_nodes_capacity is too small!");
  scene->bvh_nodes_count = 0;
  for (unsigned int mp = 0; mp < scene->mesh_primitives_count; ++mp) {
    scene->mesh_primitives[mp].BVH_index = scene->bvh_nodes_count;

    tmps.swaps_lut =
        Arena_alloc(arena, sizeof(unsigned int) * tmps.bvh_roots[mp].count);

    BVH_build(scene->bvh_nodes, &scene->bvh_nodes_count, tmps.swaps_lut,
              scene->triangles, tmps.bvh_roots[mp].first,
              tmps.bvh_roots[mp].count, BVHStrategy_get[strategy]);
  }

  Arena_rewind(as);
  Scene_set_meshes_bounds(scene);
}
