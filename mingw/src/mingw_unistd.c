/*
 * Backround process/daemon creation implementation in place of fork. Windows
 * has no fork but can be replaced with other means to achieve similar effect
 * as fork.
 */
#include <windows.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

//#define UNISTD_API_LOG
//#define UNISTD_API_ERROR_LOG

#ifdef UNISTD_API_LOG
#define UNISTD_API_PRINTF(format, ...)      MINGW_DEBUG_LOG(format, __VA_ARGS__)
#define RUN_CMD_PROC_TYPE                   CREATE_NEW_CONSOLE
#else
#define UNISTD_API_PRINTF(format, ...)      do {} while(0)
#define RUN_CMD_PROC_TYPE                   CREATE_NO_WINDOW
#endif /* #ifdef UNISTD_API_LOG */
#ifdef UNISTD_API_ERROR_LOG
#define UNISTD_API_LOG_IF_ERROR(rc)         \
        do {if(rc == -1){MINGW_ERROR_LOG("rc: %d errno: %d GetLastError: %u\n", (int)rc, errno, (unsigned)GetLastError());}} while(0)
#else
#define UNISTD_API_LOG_IF_ERROR(rc)         do {} while(0)
#endif /* #ifdef UNISTD_API_ERROR_LOG */

int run_cmd_background(char *argv[], pid_t *pid) {
    int rc = 0;
    int win_rc;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    char cmd_line[2048];
    char *cmd_line_ptr = cmd_line;
    size_t cmd_line_len = 0;
    size_t arg_len = 0;
    size_t rem_line_len = sizeof(cmd_line) - 1;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    while((*argv != NULL) && (cmd_line_len < sizeof(cmd_line))) {
        arg_len = snprintf(cmd_line_ptr, rem_line_len, "%s ", *argv);
        cmd_line_ptr += arg_len;
        rem_line_len -= arg_len;
        argv++;
    }
    /* Remove the trailing ' ' */
    *(cmd_line_ptr - 1) = '\0';

    UNISTD_API_PRINTF("\"%s\" {\n", cmd_line);

    win_rc = CreateProcess(NULL, cmd_line, NULL, NULL, FALSE,
                           RUN_CMD_PROC_TYPE, NULL, NULL, &si, &pi);
    if(0 == win_rc) {
        rc = -1;
    }

    if(pid != NULL) {
        *pid = (pid_t)pi.dwProcessId;
    }

    // Close process and thread handles.
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );

    UNISTD_API_PRINTF("\"%s\" } %d\n", cmd_line, rc);

    UNISTD_API_LOG_IF_ERROR(rc);

    return rc;
}

int daemonize_cmd(char *argv[], const char *env_name) {
    int rc = -1;
    char *env_value;

    UNISTD_API_PRINTF("\"%s\" {\n", argv[0]);

    env_value = getenv(env_name);
    /* The current process is the parent */
    if(env_value == NULL) {
        pid_t pid;

        SetEnvironmentVariable(env_name, "ON");

        rc = run_cmd_background(argv, &pid);

        UNISTD_API_PRINTF("\"%s\" } %d\n", argv[0], rc);

        /* Exit the parent process */
        exit(rc);
    }
    /* The current process is the child */
    else {
        rc = 0;
    }

    UNISTD_API_PRINTF("\"%s\" } %d\n", argv[0], rc);

    UNISTD_API_LOG_IF_ERROR(rc);

    return rc;
}

int ftruncate_win(int fd, off_t length) {
    int rc = -1;
    HANDLE hfile;

    UNISTD_API_PRINTF("%d {\n", fd);

    hfile = (HANDLE)_get_osfhandle(fd);
    if(hfile != INVALID_HANDLE_VALUE) {
        BOOL call_ok;
        LONGLONG prev_pos;
        LARGE_INTEGER result;
        LARGE_INTEGER offset;

        /* Get previous position */
        offset.QuadPart = 0;
        call_ok = SetFilePointerEx(hfile, offset, &result, FILE_CURRENT);
        if(!call_ok) {
            goto exit;
        }
        prev_pos = result.QuadPart;

        /* Set current position to length */
        offset.QuadPart = (LONGLONG)length;
        call_ok = SetFilePointerEx(hfile, offset, &result, FILE_BEGIN);
        if(!call_ok) {
            goto exit;
        }

        /* Truncate */
        call_ok = SetEndOfFile(hfile);
        if(!call_ok) {
            goto exit;
        }

        /* Set current position to previous */
        offset.QuadPart = prev_pos;
        call_ok = SetFilePointerEx(hfile, offset, &result, FILE_BEGIN);
        if(call_ok) {
            rc = 0;
        }
    }

exit:

    UNISTD_API_PRINTF("%d } %d\n", fd, rc);

    UNISTD_API_LOG_IF_ERROR(rc);

    return rc;
}

