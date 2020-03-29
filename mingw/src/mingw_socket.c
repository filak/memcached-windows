/*
 * Socket implementation for Windows. In Windows, correct APIs must be used for
 * sockets. read/write/close can't be used for sockets that's why sock_read,
 * sock_write, and sock_close APIs are added. These are just read, write, and
 * close for non-Windows build. Aside from the above note, application MUST
 * also initialize Winsock before doing socket-related calls that's why
 * sock_startup call is added to @ref main and sock_cleanup for cleanup.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <sys/socket.h>
#include <event2/event.h>
#include <poll.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

//#define SOCKET_API_LOG
//#define SOCKET_API_ERROR_LOG

#ifdef SOCKET_API_LOG
#define SOCKET_API_PRINTF(format, ...) 		MINGW_DEBUG_LOG(format, __VA_ARGS__)
#else
#define SOCKET_API_PRINTF(format, ...)		do {} while(0)
#endif /* #ifdef SOCKET_API_LOG */
#ifdef SOCKET_API_ERROR_LOG
#define SOCKET_API_LOG_IF_ERROR(sock, rc)         \
		do {if(rc == -1){MINGW_ERROR_LOG("sock:%d rc:%d errno:%d WSAGetLastError:%d\n", sock, (int)rc, errno, WSAGetLastError());}} while(0)
#define TLS_API_LOG_IF_ERROR(tls, rc)         \
		do {if(rc == -1){MINGW_ERROR_LOG("tls:%p[sock:%d] rc:%d SSL_get_error:%d errno:%d WSAGetLastError:%d\n", tls, SSL_get_fd(tls), (int)rc, SSL_get_error(tls, rc), errno, WSAGetLastError());}} while(0)
#else
#define SOCKET_API_LOG_IF_ERROR(sock, rc)			do {} while(0)
#define TLS_API_LOG_IF_ERROR(tls, rc)			    do {} while(0)
#endif /* #ifdef SOCKET_API_ERROR_LOG */

struct wsa_errno_info {
    int wsa_err;
    int errno_err;
};
static int conv_wsaerr_to_errno(int wsa_err) {
    static const struct wsa_errno_info wsa_errno_tbl[] = {
        {WSAEWOULDBLOCK,		EWOULDBLOCK},
        {WSAEBADF,				EBADF},
        {WSAECONNREFUSED,		ECONNREFUSED},
        {WSAEFAULT,				EFAULT},
        {WSAEINTR,				EINTR},
        {WSAEINVAL,				EINVAL},
        {WSA_NOT_ENOUGH_MEMORY,	ENOMEM},
        {WSAENOTCONN,			ENOTCONN},
        {WSAENOTSOCK,			ENOTSOCK},
        {WSAEACCES,				EACCES},
        {WSAEALREADY,			EALREADY},
        {WSAECONNRESET,			ECONNRESET},
        {WSAEDESTADDRREQ,		EDESTADDRREQ},
        {WSAEISCONN,			EISCONN},
        {WSAEMSGSIZE,			EMSGSIZE},
        {WSAENOBUFS,			ENOBUFS},
        {WSAEOPNOTSUPP,			EOPNOTSUPP},
        {-1,					-1},
    };
    const struct wsa_errno_info *wsa_errno_ptr = wsa_errno_tbl;

    while((wsa_errno_ptr->wsa_err != wsa_err) && (wsa_errno_ptr->wsa_err != -1)) {
        wsa_errno_ptr++;
    }

    return wsa_errno_ptr->errno_err;
}

int sock_startup(void) {
    int wsa_rc;
    WSADATA wsaData;

    SOCKET_API_PRINTF("\"%s\" {\n", GetCommandLine());

    wsa_rc = WSAStartup(MAKEWORD(2,2), &wsaData);

    SOCKET_API_PRINTF("\"%s\" } %d\n", GetCommandLine(), wsa_rc);

    SOCKET_API_LOG_IF_ERROR(0, wsa_rc);

    return wsa_rc;
}

int sock_cleanup(void) {
    int wsa_rc;

    SOCKET_API_PRINTF("\"%s\" {\n", GetCommandLine());

    wsa_rc = WSACleanup();

    SOCKET_API_PRINTF("\"%s\" } %d\n", GetCommandLine(), wsa_rc);

    SOCKET_API_LOG_IF_ERROR(0, wsa_rc);

    return wsa_rc;
}

ssize_t sendmsg(int socket, const struct msghdr *message, int flags) {
    ssize_t rc = -1;
    int wsa_ret = 0;
    DWORD bytes_sent;

    (void)flags;

    SOCKET_API_PRINTF("%d, %p, %d {\n", socket, message, flags);

    /* Based on testing, memcached will call this API even with 0 msg_iovlen.
     * WSASendTo will return error in this case. For compatibility, just
     * ignore and return success.
     */
    if(message->msg_iovlen > 0) {
        wsa_ret = WSASendTo(socket, (LPWSABUF)message->msg_iov, message->msg_iovlen,
                            &bytes_sent, 0, (const struct sockaddr *)message->msg_name, message->msg_namelen, NULL, NULL);
    } else {
        bytes_sent = 0;
    }

    if(wsa_ret != SOCKET_ERROR) {
        rc = (DWORD)bytes_sent;
    } else {
        errno = conv_wsaerr_to_errno(WSAGetLastError());
    }

    SOCKET_API_PRINTF("%d, %p, %d } %zd\n", socket, message, flags, rc);

    SOCKET_API_LOG_IF_ERROR(socket, rc);

    return rc;
}

int poll(struct pollfd *fds, nfds_t nfds, int timeout) {
    int rc = -1;

    SOCKET_API_PRINTF("%p, %lu, %d {\n", fds, nfds, timeout);

    rc = WSAPoll(fds, nfds, timeout);

    SOCKET_API_PRINTF("%p, %lu, %d } %d\n", fds, nfds, timeout, rc);

    SOCKET_API_LOG_IF_ERROR(0, rc);

    return rc;
}

#ifndef DISABLE_PIPE
int pipe(int pipefd[2]) {
    int rc = -1;
    evutil_socket_t sock_fds[2];

    SOCKET_API_PRINTF("%p[%d], %p[%d] {\n", &pipefd[0], pipefd[0], &pipefd[1], pipefd[1]);

    rc = evutil_socketpair(AF_INET, SOCK_STREAM, 0, sock_fds);
    if(rc != -1) {
        pipefd[0] = sock_fds[0];
        pipefd[1] = sock_fds[1];
    }

    SOCKET_API_PRINTF("%p[%d], %p[%d] } %d\n", &pipefd[0], pipefd[0], &pipefd[1], pipefd[1], rc);

    SOCKET_API_LOG_IF_ERROR(0, rc);

    return rc;
}

ssize_t pipe_write(int fd, const void *buf, size_t count) {
    ssize_t rc = -1;

    SOCKET_API_PRINTF("%d, %p, %zu {\n", fd, buf, count);

    rc = sock_write(fd, buf, count);

    SOCKET_API_PRINTF("%d, %p, %zu } %zd\n", fd, buf, count, rc);

    SOCKET_API_LOG_IF_ERROR(fd, rc);

    return rc;
}

ssize_t pipe_read(int fd, void *buf, size_t nbyte) {
    ssize_t rc = -1;

    SOCKET_API_PRINTF("%d, %p, %zu {\n", fd, buf, nbyte);

    rc = sock_read(fd, buf, nbyte);

    SOCKET_API_PRINTF("%d, %p, %zu } %zd\n", fd, buf, nbyte, rc);

    SOCKET_API_LOG_IF_ERROR(fd, rc);

    return rc;
}
#endif /* #ifndef DISABLE_PIPE */

ssize_t sock_write(int fd, const void *buf, size_t count) {
    ssize_t rc = -1;

    SOCKET_API_PRINTF("%d, %p, %zu {\n", fd, buf, count);

    rc = send(fd, (const char *)buf, count, 0);
    if(rc == -1) {
        errno = conv_wsaerr_to_errno(WSAGetLastError());
    }

    SOCKET_API_PRINTF("%d, %p, %zu } %zd\n", fd, buf, count, rc);

    SOCKET_API_LOG_IF_ERROR(fd, rc);

    return rc;
}

ssize_t sock_read(int fd, void *buf, size_t nbyte) {
    ssize_t rc = -1;

    SOCKET_API_PRINTF("%d, %p, %zu {\n", fd, buf, nbyte);

    rc = recv(fd, (char *)buf, nbyte, 0);
    if(rc == -1) {
        errno = conv_wsaerr_to_errno(WSAGetLastError());
    }

    SOCKET_API_PRINTF("%d, %p, %zu } %zd\n", fd, buf, nbyte, rc);

    SOCKET_API_LOG_IF_ERROR(fd, rc);

    return rc;
}

int sock_close(int fd) {
    int rc;

    SOCKET_API_PRINTF("%d {\n", fd);

    rc = closesocket(fd);
    if(SOCKET_ERROR == rc) {
        errno = conv_wsaerr_to_errno(WSAGetLastError());
    }

    SOCKET_API_PRINTF("%d } %d\n", fd, rc);

    SOCKET_API_LOG_IF_ERROR(fd, rc);

    return rc;
}

int sock_dup(int fd) {
    int rc = -1;
    BOOL call_ok;
    HANDLE hsock;

    SOCKET_API_PRINTF("%d {\n", fd);

    call_ok = DuplicateHandle(GetCurrentProcess(), (HANDLE)((intptr_t)fd),  GetCurrentProcess(), &hsock,
                              0, FALSE, DUPLICATE_SAME_ACCESS);
    if(call_ok) {
        rc = (int)((intptr_t)hsock);
    }

    SOCKET_API_PRINTF("%d } %d\n", fd, rc);

    SOCKET_API_LOG_IF_ERROR(fd, rc);

    return rc;
}

#ifdef TLS
int WinSSL_accept(SSL *ssl) {
    int rc;

    SOCKET_API_PRINTF("%p[sock:%d] {\n", ssl, SSL_get_fd(ssl));

    rc = SSL_accept(ssl);
    if(rc <= 0) {
        errno = conv_wsaerr_to_errno(WSAGetLastError());
    }

    SOCKET_API_PRINTF("%p[sock:%d] } %d\n", ssl, SSL_get_fd(ssl), rc);

    TLS_API_LOG_IF_ERROR(ssl, rc);

    return rc;
}

int WinSSL_connect(SSL *ssl) {
    int rc;

    SOCKET_API_PRINTF("%p[sock:%d] {\n", ssl, SSL_get_fd(ssl));

    rc = SSL_connect(ssl);
    if(rc < 0) {
        errno = conv_wsaerr_to_errno(WSAGetLastError());
    }

    SOCKET_API_PRINTF("%p[sock:%d] } %d\n", ssl, SSL_get_fd(ssl), rc);

    TLS_API_LOG_IF_ERROR(ssl, rc);

    return rc;
}

int WinSSL_write(SSL *ssl, const void *buf, int num) {
    int rc;

    SOCKET_API_PRINTF("%p[sock:%d], %p, %d {\n", ssl, SSL_get_fd(ssl), buf, num);

    rc = SSL_write(ssl, buf, num);
    if(rc < 0) {
        errno = conv_wsaerr_to_errno(WSAGetLastError());
    }

    SOCKET_API_PRINTF("%p[sock:%d], %p, %d } %d\n", ssl, SSL_get_fd(ssl), buf, num, rc);

    TLS_API_LOG_IF_ERROR(ssl, rc);

    return rc;
}

int WinSSL_read(SSL *ssl, void *buf, int num) {
    int rc;

    SOCKET_API_PRINTF("%p[sock:%d], %p, %d {\n", ssl, SSL_get_fd(ssl), buf, num);

    rc = SSL_read(ssl, buf, num);
    if(rc < 0) {
        errno = conv_wsaerr_to_errno(WSAGetLastError());
    }

    SOCKET_API_PRINTF("%p[sock:%d], %p, %d } %d\n", ssl, SSL_get_fd(ssl), buf, num, rc);

    TLS_API_LOG_IF_ERROR(ssl, rc);

    return rc;
}
#endif

int fcntl(int fd, int cmd, ... /* arg */ ) {
    int rc = -1;
    va_list ap;
    int param;

    SOCKET_API_PRINTF("%d, %d {\n", fd, cmd);

    va_start(ap, cmd);

    switch (cmd) {
    case F_SETFL:
        param = va_arg(ap, int);
        if(param & O_NONBLOCK) {
            u_long flags = 1;
            rc = ioctlsocket(fd, FIONBIO, &flags);
        }
        break;
    case F_GETFL:
        // Just return success
        rc = 0;
        break;
    default:

        break;
    }

    va_end(ap);

    SOCKET_API_PRINTF("%d, %d } %d\n", fd, cmd, rc);

    SOCKET_API_LOG_IF_ERROR(fd, rc);

    return rc;
}

