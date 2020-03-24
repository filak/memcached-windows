#ifndef CONN_MAP_H_INCLUDED
#define CONN_MAP_H_INCLUDED

#include "memcached.h"

int conn_list_init(int conns_max);
conn *conn_list_open(int sfd);
void conn_list_close(conn *con_ptr);

#endif // CONN_MAP_H_INCLUDED
