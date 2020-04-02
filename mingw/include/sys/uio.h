#ifndef UIO_H_INCLUDED
#define UIO_H_INCLUDED

#include <windows.h>

#define IOV_MAX 1024

/* Structure for scatter/gather I/O. which MUST be same as WSABUF structure  */
struct iovec {
    ULONG iov_len;	/* Length of data same as WSABUF's len  */
    void *iov_base;	/* Pointer to data same as WSABUF's buf.  */
};

ssize_t pread(int fd, void *buf, size_t count, off_t offset);
ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset);
ssize_t preadv(int fd, const struct iovec *iov, int iovcnt, off_t offset);

#endif // UIO_H_INCLUDED

