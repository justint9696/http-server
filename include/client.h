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
client_send(client_t *cl, const void *data, int32_t len);

int32_t
client_recv(client_t *cl, void *data, int32_t *len);

#endif // _CLIENT_H
