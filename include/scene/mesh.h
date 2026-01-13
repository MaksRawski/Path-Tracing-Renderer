#ifndef SCENE_MESH_H_
#define SCENE_MESH_H_

#include "mat4.h"

// Mesh is just a list of MeshPrimitive's, which contain a triangle list and a
// Material to use for those triangles.
//
// MeshInstance is what will actually be rendered, it puts a given mesh through
// a provided transformation into the rendered world.

typedef struct {
  unsigned int BVH_index;
  unsigned int mat_index;
  long _;
} MeshPrimitive;

typedef struct {
  unsigned int mesh_primitive_first, mesh_primitive_count;
  // TODO: 
  vec3 aabbMin, aabbMax;
  long _;
} Mesh;

typedef struct {
  // local to world transformation
  Mat4 transform;
  // world to local transformation
  Mat4 inv_transform;
  unsigned int mesh_index;
  long _;
} MeshInstance;

#endif // SCENE_MESH_H_
