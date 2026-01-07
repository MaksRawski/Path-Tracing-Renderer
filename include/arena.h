#ifndef ARENA_H_
#define ARENA_H_

#include <stddef.h>
#include <stdint.h>

typedef struct {
  uint8_t *data;
  size_t offset;
  size_t capacity;
} Arena;

Arena Arena_new(size_t capacity);

void Arena_resize(Arena *arena, size_t new_capacity);
void *Arena_alloc(Arena *arena, size_t size);

void Arena_delete(Arena *arena);

#endif // ARENA_H_
