#ifndef MINGW_LOGGER_H_INCLUDED
#define MINGW_LOGGER_H_INCLUDED

//#define MINGW_USE_SYSLOG
//#define MINGW_DEBUG

#ifdef MINGW_USE_SYSLOG
#include <syslog.h>
#define MINGW_PRINTF_DEBUG(format, ...)    syslog(LOG_INFO, format, ##__VA_ARGS__)
#define MINGW_PRINTF_ERROR(format, ...)    syslog(LOG_ERR, format, ##__VA_ARGS__)
#else
#include <stdio.h>
#define MINGW_PRINTF_DEBUG(format, ...)    fprintf (stdout, format, ##__VA_ARGS__)
#define MINGW_PRINTF_ERROR(format, ...)    fprintf (stderr, format, ##__VA_ARGS__)
#endif /* #ifdef MINGW_USE_SYSLOG */

#ifdef _WIN32
#ifdef MINGW_DEBUG
#define MINGW_DEBUG_LOG(format, ...)    \
        MINGW_PRINTF_DEBUG("[%10u] %u:%u %s " format, (unsigned)GetTickCount(), (unsigned)GetCurrentProcessId(), (unsigned)GetCurrentThreadId(), __func__, ##__VA_ARGS__)
#else
#define MINGW_DEBUG_LOG(format, ...) do {} while(0)
#endif /* #ifdef MINGW_DEBUG */
#define MINGW_ERROR_LOG(format, ...)    \
        MINGW_PRINTF_ERROR ("[%10u] %u:%u %s " format, (unsigned)GetTickCount(), (unsigned)GetCurrentProcessId(), (unsigned)GetCurrentThreadId(), __func__, ##__VA_ARGS__)
#else
#ifdef MINGW_DEBUG
#define MINGW_DEBUG_LOG(format, ...)    \
        MINGW_PRINTF_DEBUG("%ld:%lu %s " format, (long)getpid(), (unsigned long)pthread_self(), __func__, ##__VA_ARGS__)
#else
#define MINGW_DEBUG_LOG(format, ...) do {} while(0)
#endif /* #ifdef MINGW_DEBUG */
#define MINGW_ERROR_LOG(format, ...)    \
        MINGW_PRINTF_ERROR ("%ld:%lu %s " format, (long)getpid(), (unsigned long)pthread_self(), __func__, ##__VA_ARGS__)
#endif /* #ifdef _WIN32 */

#endif // MINGW_LOGGER_H_INCLUDED
