#ifndef UTILS_H_
#define UTILS_H_

#include <stdbool.h>

char *File_read(const char *filename);
bool FilePath_exists(const char *path);
const char *FilePath_get_file_name(const char *path);

#endif // UTILS_H_
