#ifndef SCENE_H_
#define SCENE_H_

#include <assert.h>
#include <stddef.h>

#include "scene/bvh.h"
#include "scene/camera.h"
#include "scene/material.h"
#include "scene/mesh.h"
#include "scene/tlas.h"
#include "scene/triangle.h"

// NOTE: this is a flat SOA (no nested arrays)
typedef struct {
  Triangle *triangles;
  BVHNode *bvh_nodes;
  Material *mats;
  MeshPrimitive *mesh_primitives;
  Mesh *meshes;
  MeshInstance *mesh_instances;
  TLASNode *tlas_nodes;
  unsigned int triangles_count, bvh_nodes_count, mats_count,
      mesh_primitives_count, last_mesh_index, mesh_instances_count,
      tlas_nodes_count;
  size_t triangles_capacity, bvh_nodes_capacity, mats_capacity,
      mesh_primitives_capacity, meshes_capacity, mesh_instances_capacity,
      tlas_nodes_capacity;

  Camera camera;
} Scene;

#define OPENGL_CHECK_ARRAY_ELEMENT_SIZE(_type)                                 \
  static_assert(sizeof(_type) % 16 == 0,                                       \
                #_type " should have a size that's a multiple of 16 bytes to " \
                       "be passed in an array to OpenGL")

OPENGL_CHECK_ARRAY_ELEMENT_SIZE(Triangle);
OPENGL_CHECK_ARRAY_ELEMENT_SIZE(BVHNode);
OPENGL_CHECK_ARRAY_ELEMENT_SIZE(Material);
OPENGL_CHECK_ARRAY_ELEMENT_SIZE(MeshPrimitive);
OPENGL_CHECK_ARRAY_ELEMENT_SIZE(Mesh);
OPENGL_CHECK_ARRAY_ELEMENT_SIZE(MeshInstance);
OPENGL_CHECK_ARRAY_ELEMENT_SIZE(TLASNode);

// NOTE: any Scene loading
Scene Scene_load_gltf(const char *path);

Scene Scene_empty(void);
bool Scene_is_empty(const Scene *scene);

void Scene_delete(Scene *self);

#endif // SCENE_H_
