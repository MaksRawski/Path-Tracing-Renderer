#ifndef FILE_PATH_H_
#define FILE_PATH_H_

#include "const_string.h"
#include <stdbool.h>

typedef struct {
  ConstString file_path;
} FilePath;

FilePath FilePath_empty(void);

// Creates a new FilePath "object" with owned copy of path
FilePath FilePath_new_copy(const char *path);

// Creates a new FilePath "object" with referenced path
FilePath FilePath_new_with(const char *path);

// Replaces the value inside FilePath with the value COPIED from argument
void FilePath_replace_copy(FilePath *self, const char *str);

// Replaces the value inside FilePath with the given pointer
void FilePath_replace_with(FilePath *self, const char *str);

bool FilePath_exists(FilePath self);
bool FilePath_eq(FilePath a, FilePath b);

void FilePath_delete(FilePath *self);

typedef struct {
  ConstString file_name;
} FileName;

// Creates a new FilePath "object" with owned copy of file_name
FileName FileName_new_copy(const char *file_name);

// Creates a new FilePath "object" with referenced file_name
FileName FileName_new_with(const char *file_name);

FileName FilePath_get_filename(FilePath self);

#endif // FILE_PATH_H_
