#include "arena.h"
#include "asserts.h"
#include "utils.h"
#include <stdlib.h>

Arena Arena_new(size_t capacity) {
  uint8_t *data = malloc(capacity);
  if (data == NULL) {
    ERROR_FMT("Failed to create an Arena with capacity of %d bytes.", capacity);
  }
  return (Arena){.capacity = capacity, .offset = 0, .data = data};
}

static inline bool Arena_is_full(Arena *arena) {
  return arena->offset == arena->capacity;
}

static inline void Arena_set_full(Arena *arena) {
  arena->offset = arena->capacity;
}

#define MAX(_a, _b) _a > _b ? _a : _b

// In case when doing an alloc would fill the Arena, dispatch the alloc call
// recursively to the next_arena. next_arena will be allocated at the end of the
// current Arena and will be created with capacity that's the bigger of:
// the size of the alloc or the current capacity.
void *Arena_alloc(Arena *arena, size_t size) {
  size_t required_capacity = arena->offset + size + sizeof(Arena);
  if (required_capacity > arena->capacity) {
    if (arena->next_arena == NULL) {
      arena->next_arena = Arena_alloc(arena, sizeof(Arena));
      *arena->next_arena = Arena_new(
          MAX(next_power_of_2(size + sizeof(Arena)), arena->capacity));
      Arena_set_full(arena);
    }
    return Arena_alloc(arena->next_arena, size);
  }
  uint8_t *ptr = arena->data + arena->offset;
  arena->offset += size;
  return ptr;
}

ArenaMark Arena_mark(Arena *arena) {
  if (Arena_is_full(arena))
    return Arena_mark(arena->next_arena);
  else
    return (ArenaMark){.arena = arena, .offset = arena->offset};
}

void Arena_rewind(ArenaMark arena_mark) {
  arena_mark.arena->offset = arena_mark.offset;
  for (Arena *a = arena_mark.arena->next_arena; a != NULL; a = a->next_arena)
    a->offset = 0;
}

void Arena_delete(Arena *arena) { free(arena->data); }
