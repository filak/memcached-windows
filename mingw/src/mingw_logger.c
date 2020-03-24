/*
 * MinGW-logging related. Just including syslog if @ref MINGW_USE_SYSLOG is enabled.
 */
#include "mingw_logger.h"

#ifdef MINGW_USE_SYSLOG
#include "syslog-client.c"
#endif /* #ifdef MINGW_USE_SYSLOG */

