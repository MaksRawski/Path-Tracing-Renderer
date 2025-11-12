#include "scene.h"
#include "file_formats/gltf.h"
#include "scene/bvh.h"
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
  BVH_apply_swaps_lut(b_res.swaps_lut, self->primitives, Primitive, self->triangles_count);
  self->bvh = b_res.bvh;

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
