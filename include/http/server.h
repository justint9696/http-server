#ifndef _HTTP_SERVER_H
#define _HTTP_SERVER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct _server {
    int32_t     sockfd;
    int32_t     port;
    char        dirname[1024];
} server_t;

int32_t
server_init(server_t *sv, int32_t port, const char *dirname);

int32_t
server_destroy(server_t *sv);

int32_t
server_accept(server_t *sv, int *fd);

int32_t
server_send(server_t *sv, int fd, const void *data, int32_t len);

int32_t
server_recv(server_t *sv, int fd, void *data, int32_t len);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _HTTP_SERVER_H
