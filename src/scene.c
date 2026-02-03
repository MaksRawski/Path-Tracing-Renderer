#include "scene.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

Scene Scene_default(void) { return (Scene){0}; }

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
