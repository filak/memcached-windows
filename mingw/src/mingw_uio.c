/*
 * UIO implementation used in extstore.
 */
#include <sys/uio.h>
#include <unistd.h>

//#define UIO_API_LOG
//#define UIO_API_ERROR_LOG

#ifdef UIO_API_LOG
#define UIO_API_PRINTF(format, ...) 		MINGW_DEBUG_LOG(format, __VA_ARGS__)
#else
#define UIO_API_PRINTF(format, ...)		    do {} while(0)
#endif /* #ifdef UIO_API_LOG */
#ifdef UIO_API_ERROR_LOG
#define UIO_API_LOG_IF_ERROR(fd, rc)         \
		do {if(rc < 0){MINGW_ERROR_LOG("fd:%d rc:%d errno:%d GetLastError:%u\n", fd, (int)rc, errno, (unsigned)GetLastError());}} while(0)
#else
#define UIO_API_LOG_IF_ERROR(fd, rc)	    do {} while(0)
#endif /* #ifdef UIO_API_ERROR_LOG */

static LONGLONG lseek_win(HANDLE hfile, LONGLONG offset, int whence);

ssize_t preadv(int fd, const struct iovec *iov, int iovcnt, off_t offset) {
    ssize_t rc = 0;
    DWORD ret_bytes;
    int i;
    HANDLE hfile;
    BOOL call_ok;

    UIO_API_PRINTF("%d, %p, %d, %lld {\n", fd, iov, iovcnt, (LONGLONG)offset);

    hfile = (HANDLE)_get_osfhandle(fd);
    lseek_win(hfile, (LONGLONG) offset, FILE_BEGIN);

    for(i = 0; i < iovcnt; i++) {
        if(iov[i].iov_len > 0) {
            call_ok = ReadFile(hfile, iov[i].iov_base, iov[i].iov_len, &ret_bytes, NULL);
            /* Return error if any of the reads has error */
            if(!call_ok) {
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
    BOOL call_ok;
    HANDLE hfile;
    DWORD ret_bytes = 0;

    UIO_API_PRINTF("%d, %p, %zu, %zu {\n", fd, buf, count, (size_t)offset);

    hfile = (HANDLE)_get_osfhandle(fd);
    lseek_win(hfile, (LONGLONG) offset, FILE_BEGIN);

    call_ok = ReadFile(hfile, buf, count, &ret_bytes, NULL);
    if(call_ok) {
        rc = (ssize_t)ret_bytes;
    }

    UIO_API_PRINTF("%d, %p, %zu, %zu } %zd\n", fd, buf, count, (size_t)offset, rc);

    UIO_API_LOG_IF_ERROR(fd, rc);

    return rc;
}

ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset) {
    ssize_t rc = -1;
    BOOL call_ok;
    HANDLE hfile;
    DWORD ret_bytes = 0;

    UIO_API_PRINTF("%d, %p, %zu, %zu {\n", fd, buf, count, (size_t)offset);

    hfile = (HANDLE)_get_osfhandle(fd);
    lseek_win(hfile, (LONGLONG) offset, FILE_BEGIN);

    call_ok = WriteFile(hfile, buf, count, &ret_bytes, NULL);
    if(call_ok) {
        rc = (ssize_t)ret_bytes;
    }

    UIO_API_PRINTF("%d, %p, %zu, %zu } %zd\n", fd, buf, count, (size_t)offset, rc);

    UIO_API_LOG_IF_ERROR(fd, rc);

    return rc;
}

static LONGLONG lseek_win(HANDLE hfile, LONGLONG offset, int whence) {
    LARGE_INTEGER result = {0};
    LARGE_INTEGER win_offset = {0};

    win_offset.QuadPart = offset;
    SetFilePointerEx(hfile, win_offset, &result, whence);

    return result.QuadPart;
}

