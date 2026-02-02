#ifndef _SERVER_H
#define _SERVER_H

#include <stdint.h>

#define PKT_LEN     1024

enum _packet_type {
    PKT_UNKOWN,
    PKT_MSG,
};

typedef struct _packet {
    int32_t     sockfd;
    int32_t     type;
    int32_t     len;
    uint8_t     data[PKT_LEN];
} packet_t;

/**
 * a server should serve multiple clients.
 * a server should create a thread for each client.
 * a server will have a lock for access to it's data.
 */
typedef struct _server {
    int32_t     sockfd;
    int32_t     port;
} server_t;

int32_t
server_init(server_t *server, int32_t port);

int32_t
server_destroy(server_t *server);

int32_t
server_accept(server_t *server, int *fd);

int32_t
server_send(server_t *server, int fd, packet_t *pkt);

int32_t
server_recv(server_t *server, int fd, packet_t *pkt);

#endif // _SERVER_H
