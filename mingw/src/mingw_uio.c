/*
 * UIO implementation used in extstore.
 */
#include <sys/uio.h>
#include <unistd.h>

//#define UIO_API_LOG
//#define UIO_API_ERROR_LOG

#ifdef UIO_API_LOG
#define UIO_API_PRINTF(format, ...)         MINGW_DEBUG_LOG(format, __VA_ARGS__)
#else
#define UIO_API_PRINTF(format, ...)         do {} while(0)
#endif /* #ifdef UIO_API_LOG */
#ifdef UIO_API_ERROR_LOG
#define UIO_API_LOG_IF_ERROR(fd, rc)         \
        do {if(rc < 0){MINGW_ERROR_LOG("fd:%d rc:%d errno:%d GetLastError:%u\n", fd, (int)rc, errno, (unsigned)GetLastError());}} while(0)
#else
#define UIO_API_LOG_IF_ERROR(fd, rc)        do {} while(0)
#endif /* #ifdef UIO_API_ERROR_LOG */

ssize_t preadv(int fd, const struct iovec *iov, int iovcnt, off_t offset) {
    ssize_t rc = 0;
    ssize_t ret_bytes;
    int i;

    UIO_API_PRINTF("%d, %p, %d, %lld {\n", fd, iov, iovcnt, (LONGLONG)offset);

    _lseeki64(fd, offset, SEEK_SET);

    for(i = 0; i < iovcnt; i++) {
        if(iov[i].iov_len > 0) {
            ret_bytes = read(fd, iov[i].iov_base, iov[i].iov_len);
            /* Return error if any of the reads has error */
            if(ret_bytes < 0) {
                rc = -1;
                break;
            }
            rc += ret_bytes;
            /* Stop reading once the read bytes is lesser than requested */
            if(ret_bytes < iov[i].iov_len) {
                break;
            }
        }
    }

    UIO_API_PRINTF("%d, %p, %d, %lld } %zd\n", fd, iov, iovcnt, (LONGLONG)offset, rc);

    UIO_API_LOG_IF_ERROR(fd, rc);

    return rc;
}

ssize_t pread(int fd, void *buf, size_t count, off_t offset) {
    ssize_t rc = -1;

    UIO_API_PRINTF("%d, %p, %zu, %zu {\n", fd, buf, count, (size_t)offset);

    _lseeki64(fd, offset, SEEK_SET);

    rc = read(fd, buf, count);

    UIO_API_PRINTF("%d, %p, %zu, %zu } %zd\n", fd, buf, count, (size_t)offset, rc);

    UIO_API_LOG_IF_ERROR(fd, rc);

    return rc;
}

ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset) {
    ssize_t rc = -1;

    UIO_API_PRINTF("%d, %p, %zu, %zu {\n", fd, buf, count, (size_t)offset);

    _lseeki64(fd, offset, SEEK_SET);

    rc = write(fd, buf, count);

    UIO_API_PRINTF("%d, %p, %zu, %zu } %zd\n", fd, buf, count, (size_t)offset, rc);

    UIO_API_LOG_IF_ERROR(fd, rc);

    return rc;
}

