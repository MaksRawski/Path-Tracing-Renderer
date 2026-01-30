#include "scene.h"
#include "arena.h"
#include "asserts.h"
#include "file_formats/gltf.h"
#include "scene/bvh.h"
#include "scene/mesh.h"
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

Scene Scene_empty(void) { return (Scene){0}; }

Scene Scene_load_gltf(const char *path) {
  Scene self = {0};
  printf("Loading scene %s...\n", path);
  load_gltf_scene(&self, path);

  return self;
}

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
void Scene_build_bvh(Scene *scene, BVHStrategy find_best_split_fn_strat) {
  if (Scene_is_empty(scene))
    return;

  static Arena arena = {0};
  if (arena.capacity == 0) {
    arena = Arena_new(1024 * 256);
  }

  build_bvh_tmp tmps = {0};
  tmps.bvh_roots =
      Arena_alloc(&arena, sizeof(TriangleList) * scene->mesh_primitives_count);

  for (unsigned int mp = 0; mp < scene->mesh_primitives_count; ++mp) {
    tmps.bvh_roots[mp].first =
        scene->bvh_nodes[scene->mesh_primitives[mp].BVH_index].first;
    tmps.bvh_roots[mp].count =
        scene->bvh_nodes[scene->mesh_primitives[mp].BVH_index].count;
  }

  ASSERTQ_CUSTOM(scene->bvh_nodes_capacity > 2 * scene->triangles_count,
                 "bvh_nodes_capacity is too small!");
  scene->bvh_nodes_count = 0;
  for (unsigned int mp = 0; mp < scene->mesh_primitives_count; ++mp) {
    scene->mesh_primitives[mp].BVH_index = scene->bvh_nodes_count;
    tmps.swaps_lut =
        Arena_alloc(&arena, sizeof(unsigned int) * tmps.bvh_roots[mp].count);
    BVH_build(scene->bvh_nodes, &scene->bvh_nodes_count, tmps.swaps_lut,
              scene->triangles, tmps.bvh_roots[mp].first,
              tmps.bvh_roots[mp].count,
              BVHStrategy_get[find_best_split_fn_strat]);
    arena.offset -= tmps.bvh_roots[mp].count;
  }

  arena.offset = 0;
  Scene_set_meshes_bounds(scene);
}

bool Scene_is_empty(const Scene *scene) { return scene->triangles_count == 0; }

void Scene_delete(Scene *self) {
  free(self->triangles);
  free(self->bvh_nodes);
  free(self->mats);
  free(self->mesh_primitives);
  free(self->meshes);
  free(self->mesh_instances);
  free(self->tlas_nodes);
  self = NULL;
}
