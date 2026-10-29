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

bool File::Open(char *path, char *mode)
{
    DWORD dwCreation;
    char absPath[MAX_PATH];
    BOOL append = FALSE;
    char *p;

    this->Close();
    for (p = mode; *p != '\0'; p++)
    {
        if (*p == 'r')
        {
            this->access = GENERIC_READ;
            dwCreation = OPEN_EXISTING;
            break;
        }
        if (*p == 'w')
        {
            DeleteFileA(path);
            this->access = GENERIC_WRITE;
            dwCreation = CREATE_ALWAYS;
            break;
        }
        if (*p == 'a')
        {
            append = TRUE;
            this->access = GENERIC_WRITE;
            dwCreation = OPEN_ALWAYS;
            break;
        }
    }
    if (*p == '\0')
    {
        return false;
    }
    MakeFullPath(absPath, path);
    this->handle = CreateFileA(absPath, this->access, FILE_SHARE_READ, NULL, dwCreation,
                               FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (this->handle == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    if (append)
    {
        SetFilePointer(this->handle, 0, NULL, FILE_END);
    }
    return true;
}

void File::Close()
{
    if (this->handle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(this->handle);
        this->handle = INVALID_HANDLE_VALUE;
        this->access = 0;
    }
}

DWORD File::Read(void *buffer, DWORD size)
{
    DWORD read = 0;

    if (this->access != GENERIC_READ)
    {
        return 0;
    }
    ReadFile(this->handle, buffer, size, &read, NULL);
    return read;
}
