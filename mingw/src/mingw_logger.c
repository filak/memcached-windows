/*
 * MinGW-logging related. Just including syslog if @ref MINGW_USE_SYSLOG is enabled.
 */
#include "mingw_logger.h"

#ifdef MINGW_USE_SYSLOG
#ifdef _WIN32
#include "syslog-client.c"
#else
/* *nix libc has built-in syslog library */
#endif /* #ifdef _WIN32 */
#endif /* #ifdef MINGW_USE_SYSLOG */

