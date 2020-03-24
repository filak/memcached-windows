#ifndef SIGNAL_H_INCLUDED
#define SIGNAL_H_INCLUDED

#include <sys/types.h>

#include_next <signal.h>

/* Type of a signal handler.  */
typedef void (*sighandler_t) (int);

struct sigaction {
    int sa_flags;
    sighandler_t sa_handler;
    /* Additional set of signals to be blocked.  */
    sigset_t sa_mask;
};

int kill(unsigned int pid, int sig);
char *strsignal(int sig);
int sigaction(int signum, const struct sigaction *act,
              struct sigaction *oldact);
int sigemptyset(sigset_t *set);

#endif // SIGNAL_H_INCLUDED
