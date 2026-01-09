#ifndef SCENE_TLAS_H_
#define SCENE_TLAS_H_

#include "vec3.h"

typedef struct {
  vec3 aabbMin, aabbMax;
  // if node is a leaf then `first` denotes the mesh_instance index
  // otherwise the left child of this node
  unsigned int first, isLeaf;
  long _;
} TLASNode;

typedef struct {
  TLASNode *nodes;
  unsigned int count;
} TLAS;

#endif // SCENE_TLAS_H_
