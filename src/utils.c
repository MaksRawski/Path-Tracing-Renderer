#include "utils.h"
#include "arena.h"
#include "asserts.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#ifdef _WIN32
#define stat _stat
#endif

char *File_read(const char *filename, Arena *arena) {
  FILE *file = fopen(filename, "r");
  if (!file)
    ERROR_FMT("Could not open file %s", filename);

  fseek(file, 0, SEEK_END);
  long length = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *buffer = Arena_alloc(arena, length + 1);
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
  const char *last_path_sep = strrchr(path, '/');
  return last_path_sep ? last_path_sep + 1 : path;
}

// TODO: this could be a simple multiplication
size_t StringArray_join_len(const char *arr[], size_t arr_len,
                            const char *sep) {
  size_t total_written = 0;
  const size_t sep_len = strlen(sep);

  for (size_t i = 0; i < arr_len - 1; ++i) {
    const size_t len = strlen(arr[i]);
    total_written += len + sep_len;
  }
  size_t len = strlen(arr[arr_len - 1]);
  total_written += len + 1;

  return total_written;
}

void StringArray_join(char *out_str, const char *arr[], size_t arr_len,
                      const char *sep) {
  const size_t sep_len = strlen(sep);
  size_t total_written = 0;
  for (size_t i = 0; i < arr_len - 1; ++i) {
    const size_t len = strlen(arr[i]);
    memcpy(out_str + total_written, arr[i], len);
    total_written += len;
    memcpy(out_str + total_written, sep, sep_len);
    total_written += sep_len;
  }
  strcpy(out_str + total_written, arr[arr_len - 1]);
}

int StringArray_find_closest_match(const char *s, size_t s_len, //
                                   const char *list[], size_t list_len) {
  int match = StringArray_find_closest_match_none;
  for (unsigned int i = 0; i < list_len; ++i) {
    if (list[i] == NULL) continue;

    bool string_is_part_of_match = true;
    for (unsigned int c = 0; c < s_len; ++c)
      string_is_part_of_match &= (tolower(s[c]) == tolower(list[i][c]));

    if (string_is_part_of_match) {
      if (match != StringArray_find_closest_match_none)
        return StringArray_find_closest_match_ambiguous;
      match = i;
    }
  }
  return match;
}

// NOTE: works only if exiftool is in PATH
void Image_add_metadata(const char *image_path, const char *description) {
  bool exiftool_available = false;
#ifdef __linux__
  exiftool_available = (system("exiftool -ver > /dev/null") == 0);
#elif defined(_WIN32)
  exiftool_available = (system("exiftool -ver >nul") == 0);
#endif
  if (exiftool_available) {
    char cmd[2048];

    int to_write = snprintf(
        cmd, sizeof(cmd), "exiftool -overwrite_original -Description='%s' '%s'",
        description, image_path);
    ASSERTQ_CUSTOM(to_write < (int)sizeof(cmd), "Buffer 'cmd' too small!");
    int cmd_result = system(cmd);
    ASSERTQ_EQ(cmd_result, 0);
  }
}

// https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
// HACK: returns 0 when x is 0
uint32_t next_power_of_2(uint32_t x) {
  --x;
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  ++x;
  return x;
}
