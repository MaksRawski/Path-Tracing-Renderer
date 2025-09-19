#ifndef SCENE_H_
#define SCENE_H_

#include "bvh.h"

typedef struct {
  Triangle *triangles;
  Material *mats;
  Primitive *primitives;
  Mesh *meshes;
  // right now only a single camera is supported
  Camera camera;

  // primitives are just wrappers around triangles so the count is going to be
  // the same
  int triangles_count, mats_count, meshes_count, camera_count;

  BVH bvh;
} Scene;

#endif // SCENE_H_
