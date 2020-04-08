/*
 * getrusage implementation for Windows.
 * Ref: https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getprocesstimes
 */
#include <sys/resource.h>
#include <windows.h>

// #define RESOURCE_API_LOG
// #define RESOURCE_API_ERROR_LOG

#ifdef RESOURCE_API_LOG
#define RESOURCE_API_PRINTF(format, ...)         MINGW_DEBUG_LOG(format, __VA_ARGS__)
#else
#define RESOURCE_API_PRINTF(format, ...)        do {} while(0)
#endif /* #ifdef RESOURCE_API_LOG */
#ifdef RESOURCE_API_ERROR_LOG
#define RESOURCE_API_LOG_IF_ERROR(rc)         \
        do {if(rc == -1){MINGW_ERROR_LOG("rc: %d errno: %d GetLastError: %u\n", (int)rc, errno, (unsigned)GetLastError());}} while(0)
#else
#define RESOURCE_API_LOG_IF_ERROR(rc)            do {} while(0)
#endif /* #ifdef RESOURCE_API_ERROR_LOG */

int getrusage(int who, struct rusage *usage) {
    int rc = 0;
    HANDLE hproc;
    BOOL call_ok;
    FILETIME create_time;
    FILETIME exit_time;
    FILETIME kernel_time;
    FILETIME user_time;

    /* Only RUSAGE_SELF is allowed in compile */
    (void)who;

    RESOURCE_API_PRINTF("%d, %p {\n", who, usage);

    hproc = GetCurrentProcess();
    call_ok = GetProcessTimes(hproc, &create_time, &exit_time, &kernel_time, &user_time);
    if(call_ok) {
        /* All times are expressed using FILETIME data structures. Such a structure
         * contains two 32-bit values that combine to form a 64-bit count of 100-nanosecond time units.
         * https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getprocesstimes
         */
        uint64_t time_val;

        time_val = ((uint64_t)kernel_time.dwHighDateTime << 32) | kernel_time.dwLowDateTime;
        usage->ru_stime.tv_sec = (long)(time_val / 10000000);
        usage->ru_stime.tv_usec = (long)(time_val % 100000000);

        time_val = ((uint64_t)user_time.dwHighDateTime << 32) | user_time.dwLowDateTime;
        usage->ru_utime.tv_sec = (long)(time_val / 10000000);
        usage->ru_utime.tv_usec = (long)(time_val % 100000000);
    } else {
        rc = -1;
    }

    RESOURCE_API_PRINTF("%d, %p[user[%ld.%06ld],system[%ld.%06ld] } %d\n", who, usage,
                        (long)usage->ru_utime.tv_sec, (long)usage->ru_utime.tv_usec,
                        (long)usage->ru_stime.tv_sec, (long)usage->ru_stime.tv_usec, rc);

    RESOURCE_API_LOG_IF_ERROR(rc);

    return rc;
}

