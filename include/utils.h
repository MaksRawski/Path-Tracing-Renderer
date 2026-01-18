#ifndef UTILS_H_
#define UTILS_H_

/* void print_triangles(Triangle *tri, vec3 *centroids, int count); */

// FILES
char *File_read(const char *filename);
#include <stdbool.h>

bool FilePath_exists(const char *path);
const char *FilePath_get_file_name(const char *path);

#endif // UTILS_H_
