#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
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

bool FilePath_exists(const char *path) {
  struct stat stats = {0};
  return stat(path, &stats) == 0;
}

const char *FilePath_get_file_name(const char *path) {
  const char *last_path_sep = strrchr(path, PATH_SEPARATOR);
  return last_path_sep ? last_path_sep + 1 : path;
}
