#ifndef UTILS_H_
#define UTILS_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// FILES
char *File_read(const char *filename);

bool FilePath_exists(const char *path);
const char *FilePath_get_file_name(const char *path);
void Image_add_metadata(const char *image_path, const char *description);

// ARRAYS OF STRINGS
bool StringArray_join(char *out_str, size_t out_str_capacity, const char *arr[],
                      size_t arr_len, const char *sep);

// NUMERIC
uint32_t next_power_of_2(uint32_t x);

#endif // UTILS_H_
