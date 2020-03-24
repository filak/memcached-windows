/*
 * Connection object management for conns array since memcached cannot
 * directly index conns using sfd in Windows to get an available
 * (either allocated or reused) connection object. memcached needs to
 * call conn_list_open first before indexing and conn_list_close to
 * release the object for later reuse. This implementation is optimized
 * without using a loop or other means that require additional runtime
 * overhead at the expense of adding 2 indeces(8 bytes) in @ref conn.
 */
#include "conn_list.h"
#include <sys/socket.h>

//#define CONN_LIST_API_LOG

#ifdef CONN_LIST_API_LOG
#define CONN_LIST_PRINTF(format, ...)        MINGW_DEBUG_LOG(format, __VA_ARGS__)
#else
#define CONN_LIST_PRINTF(format, ...)        do {} while(0)
#endif /* #ifdef CONN_LIST_API_LOG */

static int f_conn_list_max;
static int f_available_count;
static int f_next_free_idx;
static pthread_mutex_t          conns_list_lock = PTHREAD_MUTEX_INITIALIZER;
#define CONN_LIST_LOCK()        pthread_mutex_lock(&conns_list_lock)
#define CONN_LIST_UNLOCK()      pthread_mutex_unlock(&conns_list_lock)

int conn_list_init(int conns_max) {
    int rc = 0;

    CONN_LIST_PRINTF("conns_max: %d {\n", conns_max);

    f_conn_list_max = conns_max;
    f_available_count = conns_max;
    f_next_free_idx = 0;

    CONN_LIST_PRINTF("conns_max: %d } %d\n", conns_max, rc);

    return rc;
}

conn *conn_list_open(int sfd) {
    conn *con_ptr = NULL;

    CONN_LIST_LOCK();

    CONN_LIST_PRINTF("sfd: %d {\n", sfd);

    if ( f_available_count > 0 ) {
        con_ptr = conns[f_next_free_idx];

        /* Allocate connection object only for first reference.
         * This also means that all objects before conns[f_next_free_idx]
         * are already used/reused.
         */
        if(NULL == con_ptr) {
            con_ptr = (conn *)calloc(1, sizeof(conn));
            if(NULL == con_ptr) {
                CONN_LIST_PRINTF("Failed to allocate connection object! sizeof(conn): %u idx: %d max: %d\n",
                                 sizeof(conn), f_next_free_idx, f_conn_list_max);
                goto EXIT_FUNC;
            }

            con_ptr->conn_idx = f_next_free_idx;
            conns[f_next_free_idx] = con_ptr;
            con_ptr->next_free_idx = ++f_next_free_idx;
        }

        /* Associate with new sfd */
        con_ptr->sfd = sfd;

        --f_available_count;
        if (f_available_count > 0) {
            f_next_free_idx = con_ptr->next_free_idx;
        } else {
            f_next_free_idx = f_conn_list_max;
        }
    } else {
        CONN_LIST_PRINTF("Reached maximum connection limit! max: %d\n", f_conn_list_max);
    }

EXIT_FUNC:

    CONN_LIST_PRINTF("idx: %u sfd: %d } conn:%p \n", (con_ptr)?(con_ptr->conn_idx):-1, sfd, con_ptr);

    CONN_LIST_UNLOCK();

    return con_ptr;
}

void conn_list_close(conn *con_ptr) {
    CONN_LIST_LOCK();

    CONN_LIST_PRINTF("con_ptr:%p[sfd:%d] {\n", con_ptr, con_ptr->sfd);

    if (f_next_free_idx < f_conn_list_max) {
        con_ptr->next_free_idx = f_next_free_idx;
        f_next_free_idx = con_ptr->conn_idx;
    } else {
        con_ptr->next_free_idx = f_conn_list_max;
        f_next_free_idx = f_conn_list_max;
    }
    ++f_available_count;

    CONN_LIST_PRINTF("con_ptr:%p[sfd:%d] }\n", con_ptr, con_ptr->sfd);

    CONN_LIST_UNLOCK();
}

