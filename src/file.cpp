// The file class, from 0x4736c0.

#include "archive.h"

File::File()
{
    this->handle = INVALID_HANDLE_VALUE;
    this->access = 0;
}

File::~File()
{
    this->Close();
}
