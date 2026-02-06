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
      Arena_set_full(arena);
      arena->next_arena = Arena_alloc(arena, sizeof(Arena));
      *arena->next_arena =
          Arena_new(MAX(next_power_of_2(size + sizeof(Arena)), arena->capacity));
    }
    return Arena_alloc(arena->next_arena, size);
  }
  uint8_t *ptr = arena->data + arena->offset;
  arena->offset += size;
  return ptr;
}

// snapshotting and rewinding inspired by:
// https://github.com/tsoding/arena/blob/ab625dd3ac0df8c6d82cbbcd1d8fb976ecb8b9c8/arena.h#L390

ArenaSnapshot Arena_snapshot(Arena *arena) {
  if (Arena_is_full(arena))
    return Arena_snapshot(arena->next_arena);
  else
    return (ArenaSnapshot){.arena = arena, .offset = arena->offset};
}

void Arena_rewind(ArenaSnapshot snapshot) {
  snapshot.arena->offset = snapshot.offset;
  for (Arena *a = snapshot.arena->next_arena; a != NULL; a = a->next_arena)
    a->offset = 0;
}

void Arena_delete(Arena *arena) { free(arena->data); }
