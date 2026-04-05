#ifndef _RIO_STB_ALLOC_H
#define  _RIO_STB_ALLOC_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

#define KB(x) ((x) * 1024)
#define MB(x) (KB((x)) * 1024)
#define GB(x) (MB((x)) * 1024)

#pragma region ARENA_ALLOC

typedef struct {
    unsigned char* buffer;
    uint64_t buffer_size;
    uint64_t current_offset;
    uint64_t prev_offset;
} arena_t;

typedef struct {
    uint64_t n_elements;
    uint64_t size_of;
    uint64_t align;
} arena_config_align_t;

typedef struct {
    uint64_t n_elements;
    uint64_t size_of;
} arena_config_t;

bool is_pwr_two(uintptr_t x);
uintptr_t align_forward(uintptr_t ptr, size_t align);
arena_t mk_arena_allocator(uint64_t buffer_size);
void* arena_alloc_align_impl(arena_t arena[static 1], arena_config_align_t config[static 1]);
#define arena_alloc_align(arena, ...) arena_alloc_align_impl((arena), &(arena_config_align_t){__VA_ARGS__})
#ifndef DEFAULT_ALIGNMENT
#define DEFAULT_ALIGNMENT (2* sizeof(void*))
#endif
void* arena_alloc_impl(arena_t arena[static 1], arena_config_t config[static 1]);
#define arena_alloc(arena, ...) arena_alloc_impl((arena), &(arena_config_t){__VA_ARGS__})
void arena_reset_ptr(arena_t arena[static 1]);
void arena_destroy(arena_t arena[static 1]);


#pragma endregion // ARENA_ALLOC

#endif //  _RIO_STB_ALLOC_H

#ifdef RIO_STB_ALLOC_IMPLEMENTATION

#pragma region ARENA_IMPLEMENTATION


bool is_pwr_two(uintptr_t x) {
    return (x & (x-1)) == 0;
}

uintptr_t align_forward(uintptr_t ptr, size_t align) {
    uintptr_t p, a, mod;
    assert(is_pwr_two(align));

    p = ptr;
    a = (uintptr_t) align;

    mod = p & (a-1);

    if (mod != 0) {
        p += a - mod;
    }
    return p;
}

arena_t mk_arena_allocator(uint64_t buffer_size) {
    return (arena_t) {
        .buffer = malloc(buffer_size),
        .current_offset = 0,
        .prev_offset = 0,
        .buffer_size = buffer_size
    };
}

void* arena_alloc_align_impl(arena_t arena[static 1], arena_config_align_t config[static 1]) {
    uintptr_t curr_ptr = (uintptr_t) arena->buffer + (uintptr_t)arena->current_offset;
    uintptr_t offset = align_forward(curr_ptr, config->align);
    offset -= (uintptr_t) arena->buffer;

    uint64_t total_size = config->n_elements * config->size_of;
    if (offset + total_size <= arena->buffer_size) {
        void* ptr = &arena->buffer[offset];
        arena->prev_offset = offset;
        arena->current_offset = offset + total_size;

        memset(ptr, 0, total_size);
        return ptr;
    }
    return NULL;
}


void* arena_alloc_impl(arena_t arena[static 1], arena_config_t config[static 1]) {
    assert(arena && config);
    return arena_alloc_align(arena, .n_elements = config->n_elements, .size_of = config->size_of, .align = DEFAULT_ALIGNMENT);
}

void arena_reset_ptr(arena_t arena[static 1]) {
    arena->current_offset = 0;
    arena->prev_offset = 0;
    memset(arena->buffer, 0, arena->buffer_size);
}

void arena_destroy(arena_t arena[static 1]) {
    arena->current_offset = 0;
    arena->prev_offset = 0;
    arena->buffer_size = 0;
    free(arena->buffer);
}

#pragma endregion // ARENA_IMPLEMENTATION
#endif  // RIO_STB_ALLOC_IMPLEMENTATION
