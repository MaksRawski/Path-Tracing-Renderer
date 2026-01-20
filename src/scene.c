#include "scene.h"
#include "file_formats/gltf.h"
#include "scene/bvh.h"
#include "scene/bvh/strategies.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

Scene Scene_empty(void) {
  // NOTE: if counts are set to 0 then the arrays shouldn't even be accessed!
  return (Scene){0};
}

Scene Scene_load_gltf(const char *path) {
  Scene self = {0};
  printf("Loading scene %s...\n", path);
  load_gltf_scene(&self, path);

  return self;
}

void Scene_build_bvh(Scene *self,
                     BVHStrategy find_best_split_fn_strat) {
  BVHresult b_res =
      BVH_build(self->triangles, self->triangles_count,
                BVHStrategy_get[find_best_split_fn_strat]);
  BVH_apply_swaps_lut(b_res.swaps_lut, self->primitives, Primitive,
                      self->triangles_count);
  self->bvh = b_res.bvh;

  free(b_res.swaps_lut);
}

bool Scene_is_empty(const Scene *scene) { return scene->triangles_count == 0; }

void Scene_delete(Scene *self) {
  free(self->triangles);
  free(self->mats);
  free(self->primitives);
  free(self->meshes);
  BVH_delete(&self->bvh);
  self = NULL;
}
