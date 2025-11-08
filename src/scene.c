#include "scene.h"
#include "file_formats/gltf.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

void Scene_build_bvh(Scene *self);

Scene Scene_load_gltf(const char *path) {
  Scene self = {0};
  printf("Loading scene %s...\n", path);
  load_gltf_scene(&self, path);
  Scene_build_bvh(&self);

  return self;
}

void Scene_build_bvh(Scene *self) {
  BVHresult b_res = BVH_build(self->triangles, self->triangles_count);
  self->bvh = b_res.bvh;

  // swap primitives according to swaps_lut
  // NOTE: triangles are already swapped during build_bvh
  for (int t = 0; t < self->triangles_count; ++t) {
    int swap = b_res.swaps_lut[t];
    if (t < swap) {
      Primitive tmp = self->primitives[t];
      self->primitives[t] = self->primitives[swap];
      self->primitives[swap] = tmp;
    }
  }
  free(b_res.swaps_lut);
}

void Scene_delete(Scene *self) {
  free(self->triangles);
  free(self->mats);
  free(self->primitives);
  free(self->meshes);
  BVH_delete(&self->bvh);
  self = NULL;
}
