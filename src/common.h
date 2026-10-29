// Shared helpers for the decompiled code.

#pragma once

#include <stdlib.h>
#include <string.h>

// Fails to compile unless sizeof(type) == size. Generates no code.
#define ASSERT_SIZE(type, size) typedef char assert_size_##type[sizeof(type) == (size) ? 1 : -1]

// The number of elements of array `a`, as an unsigned size_t.
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

// Every free() in the game passes through a temporary, the trace an
// inlined wrapper leaves under /Ob1.
inline void MemFree(void *p)
{
    free(p);
}

// Like MemFree: malloc() through a temporary, even for a constant size.
inline void *MemAlloc(int size)
{
    return malloc(size);
}
