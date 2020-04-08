#ifndef MMAN_H_INCLUDED
#define MMAN_H_INCLUDED

#include <sys/types.h>

#define PROT_READ       0x1     /* Page can be read.  */
#define PROT_WRITE      0x2     /* Page can be written.  */
#define MAP_FAILED      NULL    /* Return value of `mmap' in case of an error. NULL for Win32's MapViewOfFileEx  */
#define MAP_SHARED      0x01    /* Share changes.  */
#define MS_SYNC         4       /* Synchronous memory sync.  */

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int munmap(void *addr, size_t length);
int msync(void *addr, size_t length, int flags);

void *alloc_large_chunk_win(const size_t limit);
int enable_large_pages_win(void);

#endif // MMAN_H_INCLUDED
