#ifndef UNISTD_H_INCLUDED
#define UNISTD_H_INCLUDED

#include_next <unistd.h>

#include "mingw_logger.h"

#define F_GETFL		3	/* Get file status flags.  */
#define F_SETFL		4	/* Set file status flags.  */
#define O_NONBLOCK	04000

int pipe(int pipefd[2]);
ssize_t pipe_write(int fd, const void *buf, size_t count);
ssize_t pipe_read(int fd, void *buf, size_t nbyte);
int run_cmd_background(char *argv[], pid_t *pid);
int daemonize_cmd(char *argv[], const char *env_name);
int ftruncate_win(int fd, off_t length);

#endif // UNISTD_H_INCLUDED
