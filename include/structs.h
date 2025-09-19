#ifndef STRUCTS_H_
#define STRUCTS_H_

#include "mat4.h"
#include "vec3.h"
#include <stdint.h>
#include <sys/types.h>

typedef struct {
  vec3 a, b, c;
  vec3 na, nb, nc;
} Triangle;

// NOTE: this struct WILL change
typedef struct {
  float emission_color[3];
  float emission_strength;
  float albedo[3];
  float specular_component;
} Material;

// binding between a triangle and a material
// NOTE: since triangles are the only supported shape, the only purpose of
// this structure is to serve as a LUT for materials
typedef struct {
  int mat;
} Primitive;

// index to first triangle of a mesh and a count of them
typedef struct {
  int index, count;
} Mesh;

// (in)fintite perspective camera
// From glTF 2.0 Specification
// (https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#cameras-overview):
// "A camera object defines the projection matrix that transforms scene
// coordinates from the view space to the clip space.
// A node containing the camera instance defines the view matrix that transforms
// scene coordinates from the global space to the view space."
typedef struct {
  // NOTE: kind of ignoring this for now
  // transforms view space to clip space
  struct ProjectionMatrix {
    float aspectRatio;
    float znear, zfar;
    float yfov;
  } projection_matrix;
  // transforms global space to view space
  Mat4 view_matrix;
} Camera;

#endif // STRUCTS_H_
