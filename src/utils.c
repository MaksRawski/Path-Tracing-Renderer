#include "utils.h"
#include "arena.h"
#include "asserts.h"

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

TinyString Time_format(double time_in_s) {
  TinyString out;
  double time_in_ms = time_in_s * 1000.0;
  double time_in_us = time_in_ms * 1000.0;
  int wanted_to_write;
  if (time_in_ms < 1.0) {
    wanted_to_write = snprintf(out.str, sizeof(out), "%.3f us", time_in_us);
  } else if (time_in_s < 1.0) {
    wanted_to_write = snprintf(out.str, sizeof(out), "%.3f ms", time_in_ms);
  } else {
    double time_in_min = time_in_s / 60;
    if (time_in_min > 1) {
      double time_in_h = time_in_min / 60;
      if (time_in_h > 1)
        wanted_to_write = snprintf(out.str, sizeof(out), "%.3f h", time_in_h);
      else
        wanted_to_write =
            snprintf(out.str, sizeof(out), "%.3f min", time_in_min);
    } else
      wanted_to_write = snprintf(out.str, sizeof(out), "%.3f s", time_in_s);
  }
  ASSERTQ_CUSTOM(wanted_to_write < (int)sizeof(out),
                 "TinyString turned out to be too small for Time_format!");
  return out;
}
