/*
 * mmap implementation for Windows. Note that in Windows, mmap works as
 * MAP_SHARED and msync is not needed or just ignored here.
 */
#include <sys/mman.h>
#include <windows.h>
#include <io.h>
#include <stdint.h>

void *mmap(void *addr, size_t length, int prot, int flags,
           int fd, off_t offset) {
    void *rc = NULL;
    DWORD page_prot = 0;
    DWORD file_access = 0;
    HANDLE hfile;
    HANDLE hmap;

    (void)flags;

    if (prot & PROT_WRITE) {
        page_prot |= PAGE_READWRITE;
    } else if (prot & PROT_READ) {
        page_prot |= PAGE_READONLY;
    }

    if (prot & PROT_READ) {
        file_access |= FILE_MAP_READ;
    }
    if (prot & PROT_WRITE) {
        file_access |= FILE_MAP_WRITE;
    }

    hfile = (HANDLE)_get_osfhandle(fd);

    hmap = CreateFileMapping(hfile, NULL, page_prot,
                             0, 0, NULL);
    if(hmap != NULL) {
        uint64_t off_64 = (uint64_t)offset;
        DWORD offlo;
        DWORD offhi;

        offlo = (DWORD)off_64;
        offhi = (DWORD)(off_64 >> 32);
        rc = MapViewOfFileEx(hmap, file_access, offhi,
                             offlo, length, addr);

        CloseHandle(hmap);
    }

    return rc;
}

int munmap(void *addr, size_t length) {
    int rc = -1;
    BOOL call_ok;

    (void)length;

    /* If the function fails, the return value is zero */
    call_ok = UnmapViewOfFile(addr);
    if(call_ok) {
        rc = 0;
    }

    return rc;
}

int msync(void *addr, size_t length, int flags) {
    int rc = -1;
    BOOL call_ok;

    (void)flags;

    call_ok = FlushViewOfFile(addr, length);
    if(call_ok) {
        rc = 0;
    }

    return rc;
}

