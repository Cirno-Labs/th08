// The game's files: a file interface and its Win32 implementation (from
// 0x4736c0), and the data archives (from 0x474860), as in th07's
// archive.h.

#pragma once

#include <windows.h>

#include "common.h"

// An abstract file, the interface File implements. Not in th06.
class FileBase
{
public:
    virtual bool Open(char *path, char *mode) = 0;
    virtual void Close() = 0;
    virtual DWORD Read(void *buffer, DWORD size) = 0;
    virtual bool Write(void *buffer, DWORD size) = 0;
    virtual DWORD Tell() = 0;
    virtual DWORD GetSize() = 0;
    virtual bool Seek(LONG offset, DWORD origin) = 0;
    virtual ~FileBase()
    {
    }
};

// A Win32 file.
class File : public FileBase
{
public:
    File();
    ~File();

    // Opens `path`, relative to the game's directory unless it has a
    // drive, in `mode` ("r", "w" or "a"). Returns true on success.
    virtual bool Open(char *path, char *mode);
    virtual void Close();
    // Returns the bytes read.
    virtual DWORD Read(void *buffer, DWORD size);
    // Returns true if all `size` bytes were written.
    virtual bool Write(void *buffer, DWORD size);
    virtual DWORD Tell();
    virtual DWORD GetSize();
    virtual bool Seek(LONG offset, DWORD origin);
    // Reads the whole file, if it is at most `maxSize` bytes, into a
    // GlobalAlloc'd buffer, and seeks back to where it was.
    virtual void *ReadAll(DWORD maxSize);

    // Reads 4 bytes into `dst`. Returns the bytes read.
    DWORD ReadDword(void *dst)
    {
        return this->Read(dst, 4);
    }

    HANDLE handle;
    // GENERIC_READ or GENERIC_WRITE.
    DWORD access;
};
ASSERT_SIZE(File, 0xc);

// FILE_BEGIN, FILE_CURRENT and FILE_END, in .rdata. Only the first is used.
extern const DWORD g_SeekOrigins[3];
// "r" and "w", the modes for File::Open. Only the first is used.
extern char *g_FileModes[2];

// Writes `path` to `dst` (MAX_PATH bytes) as a full path: in the game's
// directory unless it has a drive. Not in th06.
void MakeFullPath(char *dst, char *path);

// An entry of an archive's table.
struct DatArchiveEntry
{
    DatArchiveEntry()
    {
        this->name = NULL;
    }
    ~DatArchiveEntry()
    {
        if (this->name != NULL)
        {
            GlobalFree(this->name);
            this->name = NULL;
        }
    }

    // GlobalAlloc'd.
    char *name;
    // Where the compressed data starts in the archive; it ends where the
    // next entry's starts. The table has one more entry, without a name,
    // whose offset is the table's.
    DWORD offset;
    // The decoded size.
    DWORD size;
    DWORD checksum;
};
ASSERT_SIZE(DatArchiveEntry, 0x10);

// The header of th08.dat after the magic: three dwords, encrypted with
// DecryptData and each offset by a constant. New in th08.
struct DatArchiveHeader
{
    int numEntries;
    DWORD tableOffset;
    DWORD decodedSize;
};
ASSERT_SIZE(DatArchiveHeader, 0xc);

// Walks the decoded entry table in BuildEntryTable. New in th08; its
// inline methods are out-of-line copies at 0x475200 and 0x475220.
struct DatArchiveCursor
{
    // Moves past the dword at the cursor and returns the next one.
    DWORD SkipDword()
    {
        this->cur += 4;
        return *(DWORD *)this->cur;
    }
    // Moves past the string at the cursor and its terminator; returns
    // the new position.
    BYTE *SkipString()
    {
        this->cur += strlen((char *)this->cur) + 1;
        return this->cur;
    }

    BYTE *cur;
};

// The game's data archive (th08.dat, magic "PBGZ"), at 0x164f588: files
// are looked up by name (without the directory). As th07's, but the
// header and the table are also encrypted.
class DatArchive
{
public:
    DatArchive();
    // Calls Release.
    ~DatArchive();

    // Opens the archive at `path` and reads its table. Returns true on
    // success.
    bool Open(char *path);
    // Closes the archive and frees its table.
    void Release();
    // The decoded size of entry `name`; 0 if there is none.
    DWORD GetEntrySize(char *name);
    // Decodes entry `name` into `buffer`, which must hold GetEntrySize
    // bytes.
    void *Decode(char *name, void *buffer);
    // The entry named `name` (case-insensitive), or NULL.
    DatArchiveEntry *FindEntry(char *name);
    // Opens the file at `path` and reads the header and the table, each
    // decrypted with DecryptData. Returns true on success; on failure
    // deletes the file.
    bool ReadTable(char *path);
    // Builds the entry table from the `count` entries at `table` (each a
    // name, then offset, size and unk_0c), plus an end entry at
    // `tableOffset`. Returns a new[]'d array, or NULL.
    DatArchiveEntry *BuildEntryTable(BYTE *table, int count, DWORD tableOffset);
    // A GlobalAlloc'd copy of `str`, or NULL.
    char *DuplicateString(char *str);

    DatArchiveEntry *entries;
    int numEntries;
    // The archive's path, GlobalAlloc'd.
    char *path;
    File *file;
};
ASSERT_SIZE(DatArchive, 0x10);
extern DatArchive g_Archive;
