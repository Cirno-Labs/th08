// The constructors and destructors of the memory manager and the log,
// then App's shoot-focus flag and critical sections, 0x43ee90 to
// 0x43efef. The original's copies follow Rotate2D; split from
// unsorted.cpp at a plausible seam (nothing in the binary shows it).

#include "unsorted.h"

MemoryManager::MemoryManager()
{
    this->freeBlocksOnExit = 0;
}

MemoryManager::~MemoryManager()
{
    int i;

    if (this->freeBlocksOnExit != 0)
    {
        for (i = 0; i < 0x1000; i++)
        {
            if (this->blocks[i] != NULL)
            {
                free(this->blocks[i]);
            }
        }
    }
}

Log::Log()
{
    this->cursor = this->buffer;
    *this->cursor = '\0';
    this->hasError = 0;
}

Log::~Log()
{
}

int App::GetShootFocuses()
{
    return this->shootFocuses;
}

void App::EnterCriticalSection(int idx)
{
    ::EnterCriticalSection(&this->criticalSections[idx]);
    this->criticalSectionDepths[idx]++;
}

void App::LeaveCriticalSection(int idx)
{
    ::LeaveCriticalSection(&this->criticalSections[idx]);
    this->criticalSectionDepths[idx]--;
}
