#ifndef SCENE_H_
#define SCENE_H_

#include "scene/bvh.h"
#include "scene/camera.h"
#include "scene/material.h"
#include "scene/mesh.h"
#include "scene/primitive.h"
#include "scene/triangle.h"

typedef struct {
  Triangle *triangles;
  Material *mats;
  Primitive *primitives;
  Mesh *meshes;
  // right now only a single camera is supported
  Camera camera;

  // primitives are just wrappers around triangles so the count is going to be
  // the same
  int triangles_count, mats_count, meshes_count;

  BVH bvh;
} Scene;

Scene Scene_load_gltf(const char *path);
void Scene_delete(Scene *self);

#endif // SCENE_H_
