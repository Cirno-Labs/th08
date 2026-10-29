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

void *DatArchive::Decode(char *name, void *buffer)
{
    DatArchiveEntry *found;
    DWORD decodedSize;
    void *decoded;
    HGLOBAL packed;
    DWORD packedSize;

    packed = NULL;
    if (this->file == NULL)
    {
        return NULL;
    }
    found = this->FindEntry(name);
    if (found == NULL)
    {
        goto fail;
    }
    if (!this->file->Open(this->path, g_FileModes[0]))
    {
        goto fail;
    }
    packedSize = found[1].offset - found->offset;
    decodedSize = found->size;
    packed = GlobalAlloc(GMEM_FIXED, packedSize);
    if (packed == NULL)
    {
        goto fail;
    }
    if (!this->file->Seek(found->offset, g_SeekOrigins[0]))
    {
        goto fail;
    }
    if (this->file->Read(packed, packedSize) == 0)
    {
        goto fail;
    }
    decoded = LzssDecompress((BYTE *)packed, packedSize, (BYTE *)buffer, decodedSize);
    if (packed != NULL)
    {
        GlobalFree(packed);
        packed = NULL;
    }
    return decoded;

fail:
    DebugPrint("info : %s error\r\n", this->path);
    if (packed != NULL)
    {
        GlobalFree(packed);
        packed = NULL;
    }
    return NULL;
}

DWORD DatArchive::GetEntrySize(char *name)
{
    DatArchiveEntry *entry;

    entry = this->FindEntry(name);
    if (entry != NULL)
    {
        return entry->size;
    }
    return 0;
}
