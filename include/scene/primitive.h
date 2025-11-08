#ifndef SCENE_PRIMITIVE_H_
#define SCENE_PRIMITIVE_H_

// binding between a triangle and a material
// NOTE: since triangles are the only supported shape, the only purpose of
// this structure is to serve as a LUT for materials
typedef struct {
  int mat;
} Primitive;

#endif // SCENE_PRIMITIVE_H_
