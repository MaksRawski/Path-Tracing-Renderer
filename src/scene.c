#include "scene.h"
#include "arena.h"
#include "scene/bvh.h"
#include "scene/bvh/strategies.h"
#include <stddef.h>
#include <stdlib.h>

// NOTE: assuming that scene->bvh_nodes has enough memory already allocated
// NOTE: assuming that scene->bvh_nodes_count is 0
void Scene_build_bvh(Scene *scene, BVHStrategy find_best_split_fn_strat,
                     Arena *tmp_arena) {

  ArenaSnapshot as = Arena_snapshot(tmp_arena);
  BVHSwapsLUTElement *swaps_lut = Arena_alloc(
      tmp_arena, scene->triangles_count * sizeof(BVHSwapsLUTElement));

  scene->bvh_nodes_count = 0;
  BVH_build(scene->bvh_nodes, &scene->bvh_nodes_count, swaps_lut,
            scene->triangles, 0, scene->triangles_count,
            BVHStrategy_get[find_best_split_fn_strat], tmp_arena);

  BVH_apply_swaps_lut(swaps_lut, scene->triangles_data, TriangleEx,
                      scene->triangles_count, tmp_arena);
  Arena_rewind(as);
}

bool Scene_is_empty(const Scene *scene) { return scene->triangles_count == 0; }

void Scene_delete(Scene *self) {
  free(self->triangles);
  free(self->mats);
  free(self->triangles_data);
  free(self->bvh_nodes);
}
