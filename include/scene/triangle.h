#ifndef TRIANGLE_H_
#define TRIANGLE_H_

#include "vec3.h"

// TODO: instead move normals to a different extension structure
typedef struct {
  vec3 a, b, c;
  vec3 na, nb, nc;
} Triangle;

vec3 *Triangle_get_vertex(Triangle *t, int vertex);


#endif // TRIANGLE_H_
