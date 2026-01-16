#ifndef SCENE_TLAS_H_
#define SCENE_TLAS_H_

#include "vec3.h"
#include <stdint.h>

// if left and right are 0 then this node is a leaf
typedef struct {
  vec3 aabbMin, aabbMax;
  uint16_t left;
  uint16_t right;

  // valid only if node is a leaf
  unsigned int mesh_instance;
  long _;
} TLASNode;

#endif // SCENE_TLAS_H_
