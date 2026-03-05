//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================

#include <p3d/platform/linux/plat_filemap.hpp>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

tLinuxFileMap::tLinuxFileMap(const char* filename)
    : fh(-1)
{
    length = 0;
    Open(filename);
    SetFilename(filename);
}

tLinuxFileMap::~tLinuxFileMap()
{
    Close();
}

void
tLinuxFileMap::Open(const char* filename)
{
    fh = open(filename, O_RDONLY);
    if(fh == -1)
    {
        return;
    }

    struct stat st;
    if(fstat(fh, &st) != 0)
    {
        close(fh);
        fh = -1;
        return;
    }
    length = st.st_size;

    unsigned char *memory = (unsigned char*)mmap(NULL, length, PROT_READ, MAP_PRIVATE, fh, 0);
    if(memory == MAP_FAILED)
    {
        close(fh);
        fh = -1;
        return;
    }

    dataStream->Release();
    dataStream = new radLoadDataStream( memory, length, del );
}

void tLinuxFileMap::Close()
{
    if(GetMemory())
    {
        munmap(GetMemory(), length);
        dataStream->Release();
    }
    if(fh != -1)
    {
        close(fh);
        fh = -1;
    }
    length = 0;
}
