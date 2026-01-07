#include "scene.h"
#include "file_formats/gltf.h"
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
