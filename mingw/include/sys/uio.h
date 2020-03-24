#ifndef UIO_H_INCLUDED
#define UIO_H_INCLUDED

#define IOV_MAX 1024

/* Structure for scatter/gather I/O. which MUST be same as WSABUF structure  */
struct iovec {
    ULONG iov_len;	/* Length of data same as WSABUF's len  */
    void *iov_base;	/* Pointer to data same as WSABUF's buf.  */
};

#endif // UIO_H_INCLUDED

