#include "http/server.h"
#include "http/logger.h"
#include "http/types.h"

#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int32_t
server_init(server_t *server, int32_t port, const char *dirname) {
    struct sockaddr_in addr;

    server->port = port;

    if (dirname) {
        strcpy(server->dirname, dirname);
    }

    if ((server->sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        LOG_ERROR("server->sockfd: %s\n", strerror(errno));
        return HTTP_ERR;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server->sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        LOG_ERROR("bind: %s\n", strerror(errno));
        close(server->sockfd);
        return HTTP_ERR;
    }

    LOG_INFO("Waiting for connection on port %d...\n", server->port);
    if (listen(server->sockfd, 1) == -1) {
        LOG_ERROR("listen: %s\n", strerror(errno));
        close(server->sockfd);
        return HTTP_ERR;
    }

    return HTTP_OK;
}

int32_t
server_destroy(server_t *server) {
    close(server->sockfd);
    return HTTP_OK;
}

int32_t
server_accept(server_t *server, int *fd) {
    if ((*fd = accept(server->sockfd, NULL, NULL)) == -1) {
        LOG_ERROR("accept: %s\n", strerror(errno));
        return HTTP_ERR;
    }

    LOG_INFO("Client %d connected.\n", *fd);
    return HTTP_OK;
}

int32_t
server_send(server_t *server, int fd, const void *data, int32_t len) {
    if (send(fd, data, len, 0) == -1) {
        perror("send");
        return HTTP_ERR;
    }

    close(fd);
    return HTTP_OK;
}

int32_t
server_recv(server_t *server, int fd, void *data, int32_t len) {
    int32_t nread;
    memset(data, 0, len);
    if ((nread = recv(fd, data, len, 0)) == -1) {
        LOG_ERROR("recv: %s\n", strerror(errno));
    }

    return nread;
}
