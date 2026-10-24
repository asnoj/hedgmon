#ifndef NET_H
#define NET_H

#define MAX_CONNECTIONS 512
#define IP_STR_LEN 16
#define ADDR_STR_LEN 32
#define STATE_STR_LEN 16

typedef struct {
    char local_ip[IP_STR_LEN];
    char remote_addr[ADDR_STR_LEN];
    char state[STATE_STR_LEN];
} connection_t;

typedef struct {
    connection_t list[MAX_CONNECTIONS];
    int count;
} connection_list_t;

int net_get_tcp_connections(connection_list_t *conns);
void net_print(const connection_list_t *conns);

#endif