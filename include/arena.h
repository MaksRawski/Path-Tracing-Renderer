#ifndef ARENA_H_
#define ARENA_H_

#include <stddef.h>
#include <stdint.h>

typedef struct Arena_s {
  uint8_t *data;
  size_t offset;
  size_t capacity;
  struct Arena_s *next_arena;
} Arena;

typedef struct {
  Arena *arena;
  size_t offset;
} ArenaSnapshot;

Arena Arena_new(size_t capacity);

void *Arena_alloc(Arena *arena, size_t size);

ArenaSnapshot Arena_snapshot(Arena *arena);
void Arena_rewind(ArenaSnapshot snapshot);

void Arena_delete(Arena *arena);

#endif // ARENA_H_
