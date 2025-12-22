#ifndef SCENE_H_
#define SCENE_H_

#include <assert.h>

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
static_assert(sizeof(Triangle) % 16 == 0,
              "Triangle's size should be a multiple of 16!");
static_assert(sizeof(Material) % 16 == 0,
              "Material's size should be a multiple of 16!");
static_assert(sizeof(BVHnode) % 16 == 0,
              "BVHNode's size should be a multiple of 16!");
// NOTE: Right now primitive just contains a material index for a given
// triangle and OpenGL is fine with an array of 4-byte elements
static_assert(
    (sizeof(Primitive) == 4) || (sizeof(Primitive) % 16 == 0),
    "Primitve should contain just a single element or be a multiple of 16");

Scene Scene_load_gltf(const char *path);

Scene Scene_empty(void);
bool Scene_is_empty(const Scene *scene);

void Scene_delete(Scene *self);

#endif // SCENE_H_
