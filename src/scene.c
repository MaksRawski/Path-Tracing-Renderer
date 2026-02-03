#include "scene.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

Scene Scene_empty(void) {
  // NOTE: if counts are set to 0 then the arrays shouldn't even be accessed!
  return (Scene){0};
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
