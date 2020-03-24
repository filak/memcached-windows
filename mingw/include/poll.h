#ifndef POLL_H_INCLUDED
#define POLL_H_INCLUDED

#include <sys/socket.h>

/* Type used for the number of file descriptors.  */
typedef unsigned long int nfds_t;

int poll(struct pollfd *fds, nfds_t nfds, int timeout);

#endif // POLL_H_INCLUDED
