#ifndef SCENE_H_
#define SCENE_H_

#include "bvh.h"

#define PI 3.1415926535897932
#define DEFAULT_CAM_FOV PI / 4.0
#define DEFAULT_CAM_POS (vec3){0, 0, 0, 0}
#define DEFAULT_CAM_UP (vec3){0, 1, 0, 0}
#define DEFAULT_CAM_LOOKAT (vec3){0, 0, -1, 0}

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
