#ifndef TRIANGLE_H_
#define TRIANGLE_H_

#include "vec3.h"

typedef struct {
  vec3 a, b, c;
} Triangle;

/* typedef struct { */
/*   vec3 na, nb, nc; */
/* } TriangleNormals; */

vec3 *Triangle_get_vertex(Triangle *t, int vertex);

#endif // TRIANGLE_H_
