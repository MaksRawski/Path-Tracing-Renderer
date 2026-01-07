#include "arena.h"
#include "asserts.h"
#include "file_formats/gltf_utils.h"
#include <stdlib.h>

Arena Arena_new(size_t capacity) {
  uint8_t *data = calloc(capacity, 1);
  if (data == NULL) {
    ERROR_FMT("Failed to create an Arena with capacity of %d bytes.", capacity);
  }
  return (Arena){.capacity = capacity, .offset = 0, .data = data};
}

void Arena_resize(Arena *arena, size_t new_capacity) {
  void *new_data = realloc(arena->data, new_capacity);
  if (new_data == NULL) {
    ERROR_FMT("Failed to resize arena to capacity of %d bytes.", new_capacity);
  } else {
    arena->data = new_data;
    arena->capacity = new_capacity;
  }
}

void *Arena_alloc(Arena *arena, size_t size) {
  size_t required_capacity = arena->offset + size;
  if (required_capacity > arena->capacity) {
    Arena_resize(arena, next_power_of_2(required_capacity));
  }
  uint8_t *ptr = arena->data + arena->offset;
  arena->offset += size;
  return ptr;
}

void Arena_delete(Arena *arena) {
  free(arena->data);
  arena = NULL;
}
