#include "file_path.h"
#include "const_string.h"
#include <string.h>

#if defined(__linux__)
#include <sys/stat.h>
#elif defined(_WIN32)
#include <windows.h>
#define stat _stat
#endif

#if defined(__linux__)
#define PATH_SEPARATOR '/'
#elif defined(_WIN32)
#define PATH_SEPARATOR '\\'
#endif

FilePath FilePath_empty(void) { return FilePath_new_with("\0"); }

FilePath FilePath_new_copy(const char *path) {
  return (FilePath){.file_path = ConstString_new_copy(path)};
}

FilePath FilePath_new_with(const char *path) {
  return (FilePath){.file_path = ConstString_new_with(path)};
}

FileName FilePath_get_filename(FilePath self) {
  const char *last_path_sep = strrchr(self.file_path.str, PATH_SEPARATOR);
  const char *basename = last_path_sep ? last_path_sep + 1 : self.file_path.str;
  return FileName_new_copy(basename);
}

void FilePath_replace_with(FilePath *self, const char *str) {
  FilePath_delete(self);
  *self = FilePath_new_with(str);
}

void FilePath_replace_copy(FilePath *self, const char *str) {
  FilePath_delete(self);
  *self = FilePath_new_copy(str);
}

bool FilePath_exists(FilePath self) {
  struct stat stats = {0};
  return stat(self.file_path.str, &stats) == 0;
}

bool FilePath_eq(FilePath a, FilePath b) {
  return strcmp(a.file_path.str, b.file_path.str) == 0;
}

void FilePath_delete(FilePath *self) {
  ConstString_delete(&self->file_path);
  self = NULL;
}

// Creates a new FilePath "object" with owned copy of file_name
FileName FileName_new_copy(const char *file_name) {
  return (FileName){.file_name = ConstString_new_copy(file_name)};
}

// Creates a new FilePath "object" with referenced file_name
FileName FileName_new_with(const char *file_name) {
  return (FileName){.file_name = ConstString_new_with(file_name)};
}
