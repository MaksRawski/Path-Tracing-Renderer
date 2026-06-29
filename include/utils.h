#ifndef UTILS_H_
#define UTILS_H_

#include "arena.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// FILES
char *File_read(const char *filename, Arena *arena);

bool FilePath_exists(const char *path);
const char *FilePath_get_file_name(const char *path);
void Image_add_metadata(const char *image_path, const char *description);

// ARRAYS OF STRINGS
void StringArray_join(char *out_str, const char *arr[], size_t arr_len,
                      const char *sep);
size_t StringArray_join_len(const char *arr[], size_t arr_len, const char *sep);

/// Returns an integer in range [0, list_len) when a definite match is found
/// Returns -1 when no match is found
/// Returns -2 when there are multiple matches
/// NOTE: skips NULL pointers in list
int StringArray_find_closest_match(const char *s, size_t s_len, //
                                   const char *list[], size_t list_len);
enum StringArray_find_closest_match_err {
  StringArray_find_closest_match_none = -1,
  StringArray_find_closest_match_ambiguous = -2,
};

// NUMERIC
uint32_t next_power_of_2(uint32_t x);

#endif // UTILS_H_
