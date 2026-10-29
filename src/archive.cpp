// The data archive, from 0x474860.

#include "archive.h"
#include "lzss.h"
#include "unsorted.h"

DatArchive::DatArchive()
{
    this->entries = NULL;
    this->numEntries = 0;
    this->path = NULL;
    this->file = NULL;
}

DatArchive::~DatArchive()
{
    this->Release();
}

bool DatArchive::Open(char *path)
{
    this->Release();
    DebugPrint("info : %s open arcfile\r\n", path);
    this->file = new File();
    if (this->file == NULL)
    {
        return false;
    }
    if (this->ReadTable(path))
    {
        this->path = this->DuplicateString(path);
        if (this->path != NULL)
        {
            return true;
        }
    }
    DebugPrint("info : %s not found\r\n", path);
    this->Release();
    return false;
}

void DatArchive::Release()
{
    if (this->path != NULL)
    {
        DebugPrint("info : %s close arcfile\r\n", this->path);
    }
    if (this->path != NULL)
    {
        GlobalFree(this->path);
        this->path = NULL;
    }
    if (this->entries != NULL)
    {
        delete[] this->entries;
        this->entries = NULL;
    }
    if (this->file != NULL)
    {
        delete this->file;
        this->file = NULL;
    }
    this->numEntries = 0;
}
