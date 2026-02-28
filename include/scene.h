#ifndef SCENE_H_
#define SCENE_H_

#include <assert.h>

#include "scene/bvh.h"
#include "scene/bvh/strategies.h"
#include "scene/camera.h"
#include "scene/material.h"
#include "scene/primitive.h"
#include "scene/triangle.h"

typedef struct {
  Triangle *triangles;
  TriangleEx *triangles_data;
  BVHnode *bvh_nodes;
  Material *mats;
  Camera camera;

  uint32_t triangles_count, bvh_nodes_count, mats_count;
  uint32_t triangles_capacity, triangles_data_capacity, bvh_nodes_capacity,
      mats_capacity;
} Scene;

// NOTE: all structs that are passed as arrays to OpenGL
// must have size that's a multiple of 16
#define OPENGL_CHECK_STD430_COMPLIANCE(_type)                                  \
  static_assert(                                                               \
      sizeof(_type) == 4 || sizeof(_type) % 16 == 0, #_type                    \
      " should hold a scalar or have a size that's a multiple of 16 bytes to " \
      "be passed in an array to OpenGL")

OPENGL_CHECK_STD430_COMPLIANCE(Triangle);
OPENGL_CHECK_STD430_COMPLIANCE(Material);
OPENGL_CHECK_STD430_COMPLIANCE(TriangleEx);
OPENGL_CHECK_STD430_COMPLIANCE(Camera);
OPENGL_CHECK_STD430_COMPLIANCE(BVHnode);

inline static Scene Scene_default(void) { return (Scene){0}; }

void Scene_build_bvh(Scene *scene, BVHStrategy find_best_split_fn_strat,
                     Arena *tmp_arena);

bool Scene_is_empty(const Scene *scene);

void Scene_delete(Scene *self);

#endif // SCENE_H_
