#ifndef FILE_PATH_H_
#define FILE_PATH_H_

#include "my_string.h"
#include <stdbool.h>

// NOTE: FilePath may not actually contain a file that exists
typedef struct {
  String file_path;
} FilePath;

FilePath FilePath_new(unsigned int capacity);
FilePath FilePath_from_string(const char *path);
FilePath FilePath_of_string(char *path);
bool FilePath_exists(const FilePath *self);
bool FilePath_eq(FilePath a, FilePath b);

typedef struct {
  String file_name;
} FileName;

FileName FilePath_get_filename(const FilePath *self);
FileName FileName_from_string(const char *path);
FileName FileName_of_string(char *path);

#endif // FILE_PATH_H_
