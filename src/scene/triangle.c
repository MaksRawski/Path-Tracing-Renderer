#include "scene/triangle.h"
#include <stdio.h>
#include <stdlib.h>

// vertex must be a number from 0 to 5
vec3 *Triangle_get_vertex(Triangle *t, int vertex) {
  // if this doesn't get optimized nicely I'm going back to UB /s
  switch (vertex) {
  case 0:
    return &t->a;
  case 1:
    return &t->b;
  case 2:
    return &t->c;
  default:
    printf("Invalid triangle vertex selected: %d\n", vertex);
    exit(1);
  }
}
