#ifndef UTILS_H_
#define UTILS_H_

#include "renderer/parameters.h"
#include <stdbool.h>
#include <stddef.h>

// FILES
char *File_read(const char *filename);

bool FilePath_exists(const char *path);
const char *FilePath_get_file_name(const char *path);

void Image_add_metadata(const char *image_path,
                        const RendererParameters *renderer_parameters);

// ARRAYS OF STRINGS
bool StringArray_join(char *out_str, size_t out_str_capacity, const char *arr[],
                      size_t arr_len, const char *sep);
#endif // UTILS_H_
