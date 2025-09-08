#ifndef MIMALLOC_STUB_H
#define MIMALLOC_STUB_H

// Stub implementation for mimalloc to allow building on Linux
#include <stdlib.h>
#include <string.h>

// Fix for strdup declaration
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

char* strdup(const char* s);
char* strndup(const char* s, size_t n);

#define mi_malloc malloc
#define mi_calloc calloc
#define mi_realloc realloc
#define mi_free free
#define mi_strdup strdup
#define mi_strndup strndup
#define mi_realpath realpath
#define mi_expand(p, n) p
#define mi_recalloc(p, n, c) realloc(p, (n) * (c))
#define mi_usable_size(p) 0
#define mi_mbsdup strdup
#define mi_dupenv_s(b, n, v) (0)
#define mi_reallocf(p, n) realloc(p, n)
#define mi_malloc_good_size(sz) (sz)
#define mi_valloc malloc
#define mi_pvalloc malloc
#define mi_reallocarray(p, s, n) realloc(p, (s) * (n))
#define mi_reallocarr(p, s, n) (*(p) = realloc(*(p), (s) * (n)), 0)
#define mi_memalign(a, n) malloc(n)
#define mi_aligned_alloc(a, n) malloc(n)
#define mi_posix_memalign(p, a, n) (*(p) = malloc(n), 0)
#define mi_malloc_aligned(n, a) malloc(n)
#define mi_realloc_aligned(p, n, a) realloc(p, n)
#define mi_aligned_recalloc(p, s, n, a) realloc(p, (s) * (n))
#define mi_malloc_aligned_at(n, a, o) malloc(n)
#define mi_realloc_aligned_at(p, n, a, o) realloc(p, n)
#define mi_recalloc_aligned_at(p, s, n, a, o) realloc(p, (s) * (n))

#endif // MIMALLOC_STUB_H