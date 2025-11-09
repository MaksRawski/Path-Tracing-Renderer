#include <stdio.h>
#include <stdlib.h>

/* void print_triangles(Triangle *tri, vec3 *centroids, int count) { */
/*   for (int t = 0; t < count; ++t) { */
/*     printf("Triangle % 3d: ", t); */
/*     printf("%+f %+f %+f -- ", tri[t].a.x, tri[t].a.y, tri[t].a.z); */
/*     printf("%+f %+f %+f -- ", tri[t].b.x, tri[t].b.y, tri[t].b.z); */
/*     printf("%+f %+f %+f ", tri[t].c.x, tri[t].c.y, tri[t].c.z); */
/*     printf("(%+f %+f %+f)\n", centroids[t].x, centroids[t].y, centroids[t].z); */
/*     /\* printf("Normals     : "); *\/ */
/*     /\* printf("%+f %+f %+f -- ", tri->na.x, tri->na.y, tri->na.z); *\/ */
/*     /\* printf("%+f %+f %+f -- ", tri->nb.x, tri->nb.y, tri->nb.z); *\/ */
/*     /\* printf("%+f %+f %+f\n", tri->nc.x, tri->nc.y, tri->nc.z); *\/ */
/*   } */
/* } */

// === FILES ===

char *read_file(const char *filename) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    fprintf(stderr, "Error: Could not open file %s\n", filename);
    exit(EXIT_FAILURE);
  }

  fseek(file, 0, SEEK_END);
  long length = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *buffer = (char *)malloc(length + 1);
  if (!buffer) {
    fprintf(stderr, "Error: Could not allocate memory for file %s\n", filename);
    exit(EXIT_FAILURE);
  }

  fread(buffer, 1, length, file);
  buffer[length] = '\0';

  fclose(file);
  return buffer;
}
