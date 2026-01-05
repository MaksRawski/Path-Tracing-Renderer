#include "scene.h"
#include "asserts.h"
#include "file_formats/gltf.h"
#include "scene/bvh.h"
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
  /* Scene_build_tlas(&self); */

  return self;
}

void Scene__add_mesh(Scene *scene, unsigned int tris_first,
                     unsigned int tris_count, unsigned int mat_index) {
  if (scene->meshes_count + 1 >= scene->meshes_capacity) {
    scene->meshes_capacity *= 2;
    void *realloced = realloc(scene->meshes, scene->meshes_capacity);
    ASSERTQ_CUSTOM(realloced != NULL, "Failed to realloc meshes!");
    scene->meshes = realloced;
  }
  Mesh mesh = {.mat_index = mat_index};
  /* BVHresult res = BVH_build(scene->triangles + tris_first, tris_count); */
  scene->meshes[scene->meshes_count++] = mesh;
}

/* void Scene_build_bvh(Scene *self) { */
/*   BVHresult b_res = BVH_build(self->triangles, self->triangles_count); */
/*   BVH_apply_swaps_lut(b_res.swaps_lut, self->primitives, Primitive, */
/*                       self->triangles_count); */
/*   self->bvh = b_res.bvh; */

/*   free(b_res.swaps_lut); */
/* } */

void Scene__build_tlas(Scene *scene) {
  ASSERTQ_CUSTOM(false, "NOT YET IMPLEMENTED");
}

bool Scene_is_empty(const Scene *scene) { return scene->triangles_count == 0; }

void Scene_delete(Scene *self) {
  free(self->triangles);
  free(self->mats);
  free(self->meshes);
  self = NULL;
}
