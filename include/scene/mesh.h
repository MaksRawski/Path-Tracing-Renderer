#ifndef SCENE_MESH_H_
#define SCENE_MESH_H_

#include "mat4.h"
#include <stdint.h>

// Mesh is just a list of MeshPrimitive's, which contain a triangle list and a
// Material to use for those triangles.
//
// MeshInstance is what will actually be rendered, it puts a given mesh through
// a provided transformation into the rendered world.

typedef struct {
  uint32_t BVH_index;
  uint32_t mat_index;
  uint64_t _;
} MeshPrimitive;

typedef struct {
  vec3 aabbMin, aabbMax;
  uint32_t mesh_primitive_first, mesh_primitive_count;
  uint64_t _;
} Mesh;

typedef struct {
  // local to world transformation
  Mat4 transform;
  // world to local transformation
  Mat4 inv_transform;
  uint32_t mesh_index;
  uint64_t _;
} MeshInstance;

#endif // SCENE_MESH_H_
