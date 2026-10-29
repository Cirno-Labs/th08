// The ending's file, 0x428890 to 0x429df0, as th07's ending.cpp. Its
// inline copies of MemoryManager::Register and Unregister follow.

#include <stdlib.h>
#include <string.h>

#include "ending.h"
#include "unsorted.h"

int Ending::ReadInt()
{
    int value;

    value = atoi(this->cursor);
    while (*this->cursor != '\0')
    {
        this->cursor++;
    }
    while (*this->cursor == '\0')
    {
        this->cursor++;
    }
    return value;
}
