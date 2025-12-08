#include "file_path.h"
#include <string.h>

#if defined(__linux__)
#include <sys/stat.h>
#elif defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#define stat _stat
#endif

#if defined(__linux__)
#define PATH_SEPARATOR '/'
#elif defined(_WIN32) || defined(_WIN64)
#define PATH_SEPARATOR '\\'
#endif

FilePath FilePath_new(unsigned int capacity) {
  return (FilePath){.file_path = String_new(capacity)};
}

// Creates a new FilePath with the path set to the *pointer* given as argument
// NOTE: assumes a proper NULL terminated string
FilePath FilePath_of_string(char *path) {
  return (FilePath){.file_path = String_of(path)};
}

// Creates a new FilePath with the path set to the *value* provided as argument
FilePath FilePath_from_string(const char *path) {
  return (FilePath){.file_path = String_from(path)};
}

FileName FilePath_get_filename(const FilePath *self) {
  char *last_path_sep = strrchr(self->file_path.str, PATH_SEPARATOR);
  char *basename = last_path_sep ? last_path_sep + 1 : self->file_path.str;
  return FileName_from_string(basename);
}

bool FilePath_exists(const FilePath *self) {
  struct stat stats = {0};
  return (stat(self->file_path.str, &stats) == 0);
}

bool FilePath_eq(FilePath a, FilePath b) {
  return strcmp(a.file_path.str, b.file_path.str);
}

// Creates a new FileName with the path set to the *pointer* given as argument
// NOTE: assumes a proper NULL terminated string
FileName FileName_of_string(char *path) {
  return (FileName){.file_name = String_of(path)};
}

// Creates a new FileName with the path set to the *value* provided as
// argument
FileName FileName_from_string(const char *path) {
  return (FileName){.file_name = String_from(path)};
}
