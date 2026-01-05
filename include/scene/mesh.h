#ifndef SCENE_MESH_H_
#define SCENE_MESH_H_

#include "mat4.h"
typedef struct {
  unsigned int BVH_index;
  unsigned int mat_index;
} Mesh;

typedef struct {
  unsigned int mesh_index;
  // local-to-world transformation of the mesh
  Mat4 transform;
} MeshInstance;

#endif // SCENE_MESH_H_
