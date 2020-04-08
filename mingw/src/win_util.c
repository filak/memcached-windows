#include "win_util.h"

static void init_lsa_string(PLSA_UNICODE_STRING lsa_string, LPCWSTR orig_string);

NTSTATUS lsa_open_policy(LPWSTR server_name, DWORD access_flag, PLSA_HANDLE lsa_handle) {
    NTSTATUS rc;
    LSA_OBJECT_ATTRIBUTES obj_attr = {0};
    LSA_UNICODE_STRING lsa_string;
    PLSA_UNICODE_STRING unicode_server = NULL;

    if (server_name != NULL) {
        // Make a LSA_UNICODE_STRING out of the LPWSTR passed in
        init_lsa_string(&lsa_string, server_name);
        unicode_server = &lsa_string;
    }

    // Attempt to open the lsa_handle.
    rc = LsaOpenPolicy(unicode_server, &obj_attr, access_flag, lsa_handle);

    return rc;
}

NTSTATUS set_privilege_on_account(LSA_HANDLE lsa_handle, PSID sid_handle,
                                  LPCWSTR privilege_name, BOOL enable_flag) {
    NTSTATUS nt_status;
    LSA_UNICODE_STRING privilege_string;

    // Create a LSA_UNICODE_STRING for the privilege name.
    init_lsa_string(&privilege_string, privilege_name);

    // Grant or revoke the privilege, accordingly
    if (enable_flag) {
        nt_status = LsaAddAccountRights(lsa_handle, sid_handle, &privilege_string, 1);
    } else {
        nt_status = LsaRemoveAccountRights(lsa_handle, sid_handle, FALSE, &privilege_string, 1);
    }

    return nt_status;
}

const char *win_err2str(DWORD win_err, char *strbuf, size_t buflen) {
    size_t msglen  = 0;

    if (win_err > 0) {
        FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL, win_err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), strbuf, buflen, NULL);
        /* FormatMessageA appends \r\n, trim! */
        msglen = strlen(strbuf);
        if (msglen > 1) {
            msglen -= 2;
        }
    }

    strbuf[msglen] = '\0';

    return (const char *)strbuf;
}

static void init_lsa_string(PLSA_UNICODE_STRING lsa_string, LPCWSTR orig_string) {
    if(orig_string != NULL) {
        DWORD string_len;

        string_len = wcslen(orig_string);
        lsa_string->Buffer = (PWSTR)orig_string;
        lsa_string->Length = (USHORT)string_len * sizeof(WCHAR);
        lsa_string->MaximumLength = lsa_string->Length + sizeof(WCHAR);
    } else {
        lsa_string->Buffer = NULL;
        lsa_string->Length = 0;
        lsa_string->MaximumLength = 0;
    }
}

