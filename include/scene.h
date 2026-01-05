#ifndef SCENE_H_
#define SCENE_H_

#include <assert.h>

#include "scene/bvh.h"
#include "scene/camera.h"
#include "scene/material.h"
#include "scene/mesh.h"
#include "scene/tlas.h"
#include "scene/triangle.h"

// NOTE: any Scene loading utility is expected to only fill triangles, mats,
// the rest can be filled with calls to
// Scene__build_bvh_for_mesh and Scene__build_tlas
//
// NOTE: this is a flat SOA (no nested arrays)
typedef struct {
  Triangle *triangles;
  BVHNode *bvh_nodes;
  Material *mats;
  Mesh *meshes;
  TLASNode *tlas_nodes;
  unsigned int triangles_count, bvh_nodes_count, mats_count, meshes_count,
      tlas_nodes_count;
  unsigned int triangles_capacity, bvh_nodes_capacity, mats_capacity,
      meshes_capacity, tlas_nodes_capacity;

  Camera camera;
} Scene;

// NOTE: all structs that are passed as arrays to OpenGL
// must have size that's a multiple of 16
static_assert(sizeof(Triangle) % 16 == 0,
              "Triangle's size should be a multiple of 16!");
static_assert(sizeof(Material) % 16 == 0,
              "Material's size should be a multiple of 16!");
static_assert(sizeof(BVHNode) % 16 == 0,
              "BVHNode's size should be a multiple of 16!");

void Scene__add_mesh(Scene *scene, unsigned int tris_index,
                     unsigned int tris_count, unsigned int mat_index);

void Scene__build_tlas(Scene *scene);

// NOTE: any Scene loading
Scene Scene_load_gltf(const char *path);

Scene Scene_empty(void);
bool Scene_is_empty(const Scene *scene);

void Scene_delete(Scene *self);

#endif // SCENE_H_
