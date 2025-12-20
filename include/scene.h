#ifndef SCENE_H_
#define SCENE_H_
#include "asserts.h"

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

  // NOTE: primitives are just wrappers around triangles so the count is going
  // to be the same
  BVHTriCount triangles_count;
  unsigned int mats_count, meshes_count;

  BVH bvh;
} Scene;

// NOTE: all structs that are passed as arrays to OpenGL
// must have size that's a multiple of 16
STATIC_ASSERT(sizeof(Triangle) % 16 == 0,
              Triangle_s_size_should_be_a_multiple_of_16)
STATIC_ASSERT(sizeof(Material) % 16 == 0,
              Material_s_size_should_be_a_multiple_of_16)
STATIC_ASSERT(sizeof(BVHnode) % 16 == 0,
              BVHNode_s_size_should_be_a_multiple_of_16)
// NOTE: right now Primive has just a single field which can be
// passed as array without any issues
STATIC_ASSERT(sizeof(Primitive) == 4,
              Primitve_s_size_should_be_a_single_element)

Scene Scene_empty(void);
Scene Scene_load_gltf(const char *path);
void Scene_delete(Scene *self);

#endif // SCENE_H_
