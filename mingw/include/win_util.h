#ifndef WIN_UTIL_H_INCLUDED
#define WIN_UTIL_H_INCLUDED

#include <windows.h>
#include <ntstatus.h>
#include <ntsecapi.h>

#define WINUTIL_LOG_LASTERROR(last_err)         \
        do {                                    \
            char last_errstr[64];               \
            MINGW_ERROR_LOG("GetLastError: %u(%s)\n", (unsigned)last_err, win_err2str((ULONG)last_err, last_errstr, sizeof(last_errstr)));    \
        } while(0)

const char *win_err2str(DWORD win_err, char *strbuf, size_t buflen);
NTSTATUS lsa_open_policy(LPWSTR server_name, DWORD access_flag, PLSA_HANDLE lsa_handle);
NTSTATUS set_privilege_on_account(LSA_HANDLE lsa_handle, PSID sid_handle,
                                  LPCWSTR privilege_name, BOOL enable_flag);

#endif // WIN_UTIL_H_INCLUDED
