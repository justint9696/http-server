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
server_init(server_t *sv, int32_t port, const char *dirname) {
    struct sockaddr_in addr;

    sv->port = port;

    if (dirname) {
        strcpy(sv->dirname, dirname);
    }

    if ((sv->sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        LOG_ERROR("socket: %s\n", strerror(errno));
        return HTTP_ERR;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sv->sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        LOG_ERROR("bind: %s\n", strerror(errno));
        close(sv->sockfd);
        return HTTP_ERR;
    }

    if (listen(sv->sockfd, SOMAXCONN) == -1) {
        LOG_ERROR("listen: %s\n", strerror(errno));
        close(sv->sockfd);
        return HTTP_ERR;
    }

    return HTTP_OK;
}

int32_t
server_destroy(server_t *sv) {
    close(sv->sockfd);
    return HTTP_OK;
}

int32_t
server_accept(server_t *sv, int *fd) {
    if ((*fd = accept(sv->sockfd, NULL, NULL)) == -1) {
        LOG_INFO("accept: %s\n", strerror(errno));
        return HTTP_ERR;
    }

    return HTTP_OK;
}

int32_t
server_send(server_t *sv, int fd, const void *data, int32_t len) {
    if (send(fd, data, len, 0) == -1) {
        LOG_INFO("send: %s\n", strerror(errno));
        return HTTP_ERR;
    }

    return HTTP_OK;
}

int32_t
server_recv(server_t *sv, int fd, void *data, int32_t len) {
    int32_t nread;
    memset(data, 0, len);
    if ((nread = recv(fd, data, len, 0)) == -1) {
        LOG_INFO("recv: %s\n", strerror(errno));
    }

    return nread;
}
