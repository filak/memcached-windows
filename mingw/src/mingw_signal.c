/*
 * Additional signal implementation for Windows. C standard library already has
 * <signal.h> but it lacks *nix-specific used by memcached. Note that this
 * implementation has limitation due to Windows limitation itself. In Windows,
 * a process can't kill another process with a signal and the other process
 * can handle the kill signal (e.g. SIGTERM). Process is killed immediately via
 * @ref kill API (internally calls TerminateProcess). It can however send SIGINT
 * if a process is killed via Ctrl+C in command prompt. This will remain a
 * limitation unless new IPC mechanism is added to memcached (at least for Windows)
 * for graceful shutdown. Sending "shutdown" to memcached server ran with -A is
 * also another option for user if really necessary.
 * Ref: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/signal?view=vs-2019
 */
#include <signal.h>
#include <unistd.h>
#include <windows.h>
#include <errno.h>

//#define SIGNAL_API_LOG

#ifdef SIGNAL_API_LOG
#define SIGNAL_API_PRINTF(format, ...) 		MINGW_DEBUG_LOG(format, __VA_ARGS__)
#else
#define SIGNAL_API_PRINTF(format, ...)		do {} while(0)
#endif /* #ifdef SIGNAL_API_LOG */

int kill(unsigned int pid, int sig) {
    int rc = -1;
    HANDLE hproc;

    (void)sig;

    SIGNAL_API_PRINTF("%u %d }\n", pid, sig);

    hproc = OpenProcess(SYNCHRONIZE | PROCESS_TERMINATE, TRUE, pid);
    if(hproc != NULL) {
        BOOL ret;

        ret = TerminateProcess(hproc, 0);
        if(ret) {
            rc = 0;
        }

        CloseHandle(hproc);
    } else {
        errno = ESRCH;
    }

    SIGNAL_API_PRINTF("%u %d } %d\n", pid, sig, rc);

    return rc;
}

struct signal_info {
    const char *name;
    int sig;
};
char *strsignal(int sig) {
    static const struct signal_info sig_tbl[] = {
        {"SIGHUP",	1},
        {"SIGINT",	2},
        {"SIGQUIT", 3},
        {"SIGILL",	4},
        {"SIGTRAP", 5},
        {"SIGIOT",	6},
        {"SIGEMT",	7},
        {"SIGFPE",	8},
        {"SIGKILL", 9},
        {"SIGBUS",	10},
        {"SIGSEGV", 11},
        {"SIGSYS",	12},
        {"SIGTERM", 15},
        {"SIGBREAK",21},
        {"SIGABRT",	22},
        {"UNKNOWN",	-1}
    };
    const struct signal_info *sig_ptr = sig_tbl;

    while((sig_ptr->sig != sig) && (sig_ptr->sig != -1)) {
        sig_ptr++;
    }

    return (char *)sig_ptr->name;
}

int sigaction(int signum, const struct sigaction *act,
              struct sigaction *oldact) {
    int rc = -1;
    sighandler_t prev_handler;

    prev_handler = signal(signum, act->sa_handler);
    if(prev_handler != SIG_ERR) {
        rc = 0;
        if(oldact != NULL) {
            oldact->sa_handler = prev_handler;
        }
    }

    return rc;
}

int sigemptyset(sigset_t *set) {
    int rc = EINVAL;

    if(set != NULL) {
        memset(set, 0, sizeof(*set));
        rc = 0;
    }

    return rc;
}
