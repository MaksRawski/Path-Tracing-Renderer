#ifndef SCENE_TLAS_H_
#define SCENE_TLAS_H_

#include "vec3.h"

typedef struct {
  vec3 aabbMin, aabbMax;
  // if `count` is 1 then this node is a leaf and `first` denotes
  // the mesh index
  unsigned int first, count;
  long _;
} TLASNode;

typedef struct {
  TLASNode *nodes;
  unsigned int count;
} TLAS;

#endif // SCENE_TLAS_H_
