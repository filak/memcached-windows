/*
 * mmap implementation for Windows. Note that in Windows, mmap works as
 * MAP_SHARED and msync is not needed or just ignored here.
 */
#include <sys/mman.h>
#include <io.h>
#include <stdint.h>

#include "win_util.h"

//#define MMAN_API_LOG
//#define MMAN_API_ERROR_LOG

#ifdef MMAN_API_LOG
#define MMAN_API_PRINTF(format, ...)        MINGW_DEBUG_LOG(format, ##__VA_ARGS__)
#else
#define MMAN_API_PRINTF(format, ...)        do {} while(0)
#endif /* #ifdef MMAN_API_LOG */

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    void *rc = NULL;
    DWORD page_prot = 0;
    DWORD file_access = 0;
    HANDLE hfile;
    HANDLE hmap;

    (void)flags;

    MMAN_API_PRINTF("%p, %zu, 0x%08X, 0x%08X, %d, %lld {\n", addr, length, prot, flags, fd, (long long)offset);

    if (prot & PROT_WRITE) {
        page_prot |= PAGE_READWRITE;
    } else if (prot & PROT_READ) {
        page_prot |= PAGE_READONLY;
    }

    if (prot & PROT_READ) {
        file_access |= FILE_MAP_READ;
    }
    if (prot & PROT_WRITE) {
        file_access |= FILE_MAP_WRITE;
    }

    hfile = (HANDLE)_get_osfhandle(fd);

    hmap = CreateFileMapping(hfile, NULL, page_prot,
                             0, 0, NULL);
    if(hmap != NULL) {
        uint64_t off_64 = (uint64_t)offset;
        DWORD offlo;
        DWORD offhi;

        offlo = (DWORD)off_64;
        offhi = (DWORD)(off_64 >> 32);
        rc = MapViewOfFileEx(hmap, file_access, offhi,
                             offlo, length, addr);

        CloseHandle(hmap);
    }

    MMAN_API_PRINTF("%p, %zu, 0x%08X, 0x%08X, %d, %lld } %p\n", addr, length, prot, flags, fd, (long long)offset, rc);

    return rc;
}

int munmap(void *addr, size_t length) {
    int rc = -1;
    BOOL call_ok;

    (void)length;

    MMAN_API_PRINTF("%p, %zu {\n", addr, length);

    /* If the function fails, the return value is zero */
    call_ok = UnmapViewOfFile(addr);
    if(call_ok) {
        rc = 0;
    }

    MMAN_API_PRINTF("%p, %zu } %d\n", addr, length, rc);

    return rc;
}

int msync(void *addr, size_t length, int flags) {
    int rc = -1;
    BOOL call_ok;

    (void)flags;

    MMAN_API_PRINTF("%p, %zu, 0x%08X {\n", addr, length, flags);

    call_ok = FlushViewOfFile(addr, length);
    if(call_ok) {
        rc = 0;
    }

    MMAN_API_PRINTF("%p, %zu, 0x%08X } %d\n", addr, length, flags, rc);

    return rc;
}

void *alloc_large_chunk_win(const size_t limit) {
    void *rc = NULL;
    size_t aligned_limit;
    size_t alignment;

    MMAN_API_PRINTF("%zu {\n", limit);

    alignment = (size_t)GetLargePageMinimum();
    aligned_limit = ((limit + alignment - 1) / alignment) * alignment;
    MMAN_API_PRINTF("alignment: %zu aligned_limit: %zu\n", alignment, aligned_limit);

    rc = VirtualAlloc(NULL, aligned_limit, MEM_RESERVE | MEM_COMMIT | MEM_LARGE_PAGES, PAGE_READWRITE);

    MMAN_API_PRINTF("%zu } %p\n", limit, rc);

    return rc;
}

#define LARGE_PAGE_PRIVILEGE            "SeLockMemoryPrivilege"
#define LARGE_PAGE_PRIVILEGE_UNICODE    L"SeLockMemoryPrivilege"
int enable_large_pages_win(void) {
    int rc = -1;
    BOOL call_ok;
    HANDLE token_handle = NULL;
    TOKEN_PRIVILEGES token_priv;
    PTOKEN_USER token_ptr = NULL;
    DWORD token_bufsize = 0;
    NTSTATUS nt_status;
    DWORD last_err;
    LSA_HANDLE lsa_handle;

    MMAN_API_PRINTF(" {\n");

    call_ok = OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token_handle);
    if (!call_ok) {
        WINUTIL_LOG_LASTERROR(GetLastError());
        goto exit_func;
    }

    // Probe the buffer size reqired for PTOKEN_USER structure
    call_ok = GetTokenInformation(token_handle, TokenUser, NULL, 0, &token_bufsize);
    if (!call_ok && (GetLastError() != ERROR_INSUFFICIENT_BUFFER)) {
        WINUTIL_LOG_LASTERROR(GetLastError());
        goto exit_func;
    }
    // Retrieve the token information in a TOKEN_USER structure
    token_ptr = (PTOKEN_USER) malloc(token_bufsize);
    call_ok = GetTokenInformation(token_handle, TokenUser, token_ptr, token_bufsize, &token_bufsize);
    if (!call_ok) {
        WINUTIL_LOG_LASTERROR(GetLastError());
        goto exit_func;
    }

    // Cleanup previous token handle
    CloseHandle(token_handle);
    token_handle = NULL;

    nt_status = lsa_open_policy(NULL, POLICY_CREATE_ACCOUNT | POLICY_LOOKUP_NAMES, &lsa_handle);
    if (nt_status != STATUS_SUCCESS) {
        WINUTIL_LOG_LASTERROR(LsaNtStatusToWinError(nt_status));
        goto exit_func;
    }

    // Add new privelege to the account
    nt_status = set_privilege_on_account(lsa_handle, token_ptr->User.Sid, LARGE_PAGE_PRIVILEGE_UNICODE, TRUE);
    if (nt_status != STATUS_SUCCESS) {
        WINUTIL_LOG_LASTERROR(LsaNtStatusToWinError(nt_status));
        goto exit_func;
    }

    call_ok = OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &token_handle);
    if (!call_ok) {
        WINUTIL_LOG_LASTERROR(GetLastError());
        goto exit_func;
    }

    token_priv.PrivilegeCount = 1;
    token_priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    call_ok = LookupPrivilegeValue(NULL, LARGE_PAGE_PRIVILEGE, &token_priv.Privileges[0].Luid);
    if (!call_ok) {
        WINUTIL_LOG_LASTERROR(GetLastError());
        goto exit_func;
    }

    call_ok = AdjustTokenPrivileges(token_handle, FALSE, &token_priv, 0, (PTOKEN_PRIVILEGES)NULL, 0);
    last_err = GetLastError();
    if (!call_ok || (last_err != ERROR_SUCCESS)) {
        WINUTIL_LOG_LASTERROR(last_err);
    } else {
        /* Success if all syscalls succeed */
        rc = 0;
    }

exit_func:
    if(token_ptr) {
        free(token_ptr);
    }

    if(token_handle) {
        CloseHandle(token_handle);
    }

    MMAN_API_PRINTF(" } %d\n", rc);

    return rc;
}

