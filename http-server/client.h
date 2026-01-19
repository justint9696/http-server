#ifndef _CLIENT_H
#define _CLIENT_H

#include "server.h"

typedef struct _client {
    int32_t     sockfd;
    int32_t     portno;
} client_t;

int32_t
client_init(client_t *cl, int32_t portno);

int32_t
client_destroy(client_t *cl);

int32_t
client_send(client_t *cl, packet_t *pkt);

int32_t
client_recv(client_t *cl, packet_t *pkt);

#endif // _CLIENT_H
