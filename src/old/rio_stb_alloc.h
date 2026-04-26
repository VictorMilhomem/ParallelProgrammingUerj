#ifndef _RIO_STB_ALLOC_H
#define  _RIO_STB_ALLOC_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

// TODO: Create an arena using multiple chunks
// Current version works like [guard][arena][guard]
// with chunks would be [chunk 1] -> [chunk 2] -> [chunk 3]
// where each chunk is [guard][usable][guard]

#define KB(x) ((x) * 1024)
#define MB(x) (KB((x)) * 1024)
#define GB(x) (MB((x)) * 1024)

#pragma region ARENA_ALLOC

typedef struct {
    uint8_t* buffer;
    uint8_t* base_ptr;
    uint64_t buffer_size;
    uint64_t total_size;
    uint64_t current_offset;
    uint64_t prev_offset;
} arena_t;

typedef struct {
    uint64_t size_of;
    uint64_t align;
} arena_config_t;

typedef struct {
    arena_t* arena;
    uint64_t prev_offset;
    uint64_t current_offset;
} tmp_arena_t;

uint64_t round_up_multiple_of(uint64_t n, uint64_t multiple);
bool is_pwr_two(uintptr_t x);
uintptr_t align_forward(uintptr_t ptr, size_t align);
arena_t mk_arena_allocator(uint64_t buffer_size);
void* arena_alloc_align_impl(arena_t arena[static 1], arena_config_t config[static 1]);
#define arena_alloc_align(arena, ...) arena_alloc_align_impl((arena), &(arena_config_t){__VA_ARGS__})
#define arena_alloc(arena, type, count)               \
    (                                                 \
        assert((count) <= UINT64_MAX / sizeof(type)), \
        (type*) arena_alloc_align((arena),            \
             .size_of = sizeof(type) * count,         \
             .align = _Alignof(type))                 \
    )

tmp_arena_t mk_tmp_arena(arena_t arena[static 1]);
void tmp_arena_destroy(tmp_arena_t tmp[static 1]);
void arena_reset_ptr(arena_t arena[static 1]);
void arena_destroy(arena_t arena[static 1]);


#pragma endregion // ARENA_ALLOC

#endif //  _RIO_STB_ALLOC_H

#ifdef RIO_STB_ALLOC_IMPLEMENTATION

#pragma region ARENA_IMPLEMENTATION

uint64_t round_up_multiple_of(uint64_t n, uint64_t multiple) {
    if (multiple == 0) return n;
    return ((n + multiple - 1) / multiple) * multiple;
}

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
    uint64_t page_size = (uint64_t) sysconf(_SC_PAGE_SIZE);
    uint64_t alloc_real_size = round_up_multiple_of(buffer_size, page_size);
    uint64_t mmap_size = alloc_real_size + 2 * page_size;

    uint8_t* base = mmap(NULL, mmap_size,
                                 PROT_READ|PROT_WRITE,
                                 MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    assert(base != MAP_FAILED);
    uint8_t* buffer = base + page_size;

    int r1 = mprotect(base, page_size, PROT_NONE);
    int r2 = mprotect(buffer + alloc_real_size, page_size, PROT_NONE);
    assert(r1 == 0 && r2 == 0);

    return (arena_t) {
        .buffer = buffer,
        .base_ptr = base,
        .buffer_size = alloc_real_size,
        .total_size = mmap_size,
        .current_offset = 0,
        .prev_offset = 0,
    };
}

void* arena_alloc_align_impl(arena_t arena[static 1], arena_config_t config[static 1]) {
    assert(arena && config);
    uintptr_t curr_ptr = (uintptr_t) arena->buffer + (uintptr_t)arena->current_offset;
    uintptr_t offset = align_forward(curr_ptr, config->align);
    offset -= (uintptr_t) arena->buffer;

    if (offset + config->size_of > arena->buffer_size) {
        return NULL;
    }
    void* ptr = &arena->buffer[offset];
    arena->prev_offset = offset;
    arena->current_offset = offset + config->size_of;

    memset(ptr, 0, config->size_of);
    return ptr;
}

void* arena_alloc_impl(arena_t arena[static 1], arena_config_t config[static 1]) {
    assert(arena && config);
    return arena_alloc_align(arena,  .size_of = config->size_of);
}

tmp_arena_t mk_tmp_arena(arena_t arena[static 1]) {
    assert(arena);
    return (tmp_arena_t) {
        .arena = arena,
        .prev_offset = arena->prev_offset,
        .current_offset = arena->current_offset
    };
}

void tmp_arena_destroy(tmp_arena_t tmp[static 1]) {
    assert(tmp);
    tmp->arena->prev_offset = tmp->prev_offset;
    tmp->arena->current_offset = tmp->current_offset;
}

void arena_reset_ptr(arena_t arena[static 1]) {
    assert(arena);
    arena->current_offset = 0;
    arena->prev_offset = 0;
    memset(arena->buffer, 0, arena->buffer_size);
}

void arena_destroy(arena_t arena[static 1]) {
    assert(arena);
    munmap(arena->buffer, arena->buffer_size);

    arena->buffer = NULL;
    arena->base_ptr = NULL;
    arena->buffer_size = 0;
    arena->total_size = 0;
    arena->current_offset = 0;
    arena->prev_offset = 0;

}

#pragma endregion // ARENA_IMPLEMENTATION
#endif  // RIO_STB_ALLOC_IMPLEMENTATION
