#include "http/server.h"

#include <string.h>

int32_t
server_init(server_t *sv, int32_t portno, const char *dirname) {
    if (dirname) {
        strcpy(sv->dirname, dirname);
    }

    return socket_bind(&sv->socket, portno);
}

inline int32_t
server_destroy(server_t *sv) {
    return socket_close(&sv->socket);
}

inline int32_t
server_accept(server_t *sv, int *fd) {
    return socket_accept(&sv->socket, fd);
}

inline int32_t
server_send(server_t *sv, int fd, const void *data, int32_t len) {
    return socket_send((socket_t *)&fd, data, len);
}

inline int32_t
server_recv(server_t *sv, int fd, void *data, int32_t len) {
    return socket_recv((socket_t *)&fd, data, len);
}
