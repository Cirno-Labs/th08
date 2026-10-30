// Shared helpers for the decompiled code.

#pragma once

#include <stdlib.h>
#include <string.h>

// Fails to compile unless sizeof(type) == size. Generates no code.
#define ASSERT_SIZE(type, size) typedef char assert_size_##type[sizeof(type) == (size) ? 1 : -1]

// The number of elements of array `a`, as an unsigned size_t.
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

// Marks a constructor that only the static initializer of a global
// calls. The standalone exe defines it inline, as the original did: its
// out-of-line copy then lands where the global is defined or where the
// constructor is, whichever comes later. The DLL build defines it out of
// line: it constructs no globals, and an inline function nobody calls
// would not be compiled, or compared.
#ifdef TH08_EXE
#define EXE_INLINE inline
#else
#define EXE_INLINE
#endif

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

// The object every allocation goes through in th08, in place of th07's
// MemAlloc and MemFree. Alloc takes a name for the block: a class name,
// "midi", or __FILE__ of an inline in global.h.
class MemoryManager
{
public:
    // Out-of-line copies of inline functions, at the end of the input
    // object (0x43ee90, 0x43eeb0); nothing references them.
    MemoryManager();
    // Frees every block left in `blocks` if `freeBlocksOnExit` is set.
    ~MemoryManager();
    // malloc() and free(); `name` is unused in the release build.
    // Inline; the copies are at 0x40b880 and 0x40b8a0.
    void *Alloc(int size, const char *name)
    {
        return malloc(size);
    }
    void Free(void *p)
    {
        free(p);
    }
    // Records `p`, a block of `size` bytes allocated with new, under
    // `name` and returns it; a no-op in the release build. Out-of-line
    // copy at 0x429de0.
    void *Register(void *p, int size, const char *name)
    {
        return p;
    }
    // Forgets `p` before it is deleted; a no-op (0x429df0).
    void Unregister(void *p)
    {
    }

    // Allocated blocks, for a debug build's leak tracking: the release
    // Alloc and Free leave it alone.
    void *blocks[0x1000];
    int freeBlocksOnExit;
};
extern MemoryManager g_MemoryManager;
