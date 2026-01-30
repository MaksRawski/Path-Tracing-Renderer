#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#ifdef _WIN32
#define stat _stat
#endif

#if defined(__linux__)
#define PATH_SEPARATOR '/'
#elif defined(_WIN32)
#define PATH_SEPARATOR '\\'
#endif

char *File_read(const char *filename) {
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
  struct stat stats;
  return stat(path, &stats) == 0;
}

const char *FilePath_get_file_name(const char *path) {
  const char *last_path_sep = strrchr(path, PATH_SEPARATOR);
  return last_path_sep ? last_path_sep + 1 : path;
}

// returns false when the result's length exceeds out_str_capacity
bool StringArray_join(char *out_str, size_t out_str_capacity, const char *arr[],
                      size_t arr_len, const char *sep) {
  // TODO: use regular memcpy
  size_t total_written = 0;
  size_t sep_len = strlen(sep);
  for (size_t i = 0; i < arr_len - 1; ++i) {
    size_t len = strlen(arr[i]);
    total_written += len + sep_len;
    if (total_written > out_str_capacity) {
      return false;
    }
    strncat(out_str, arr[i], len);
    strncat(out_str, sep, sep_len);
  }

  size_t len = strlen(arr[arr_len - 1]);
  total_written += len + 1;
  if (total_written > out_str_capacity)
    return false;

  strncat(out_str, arr[arr_len - 1], len);
  return true;
}
