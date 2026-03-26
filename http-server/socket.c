#include "http/socket.h"
#include "http/logger.h"
#include "http/types.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int32_t
socket_bind(socket_t *sck, int32_t portno) {
    struct sockaddr_in addr;

    if ((*sck = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        LOG_ERROR("socket: %s\n", strerror(errno));
        return HTTP_ERR;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(portno);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(*sck, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        LOG_ERROR("bind: %s\n", strerror(errno));
        close(*sck);
        return HTTP_ERR;
    }

    if (listen(*sck, SOMAXCONN) == -1) {
        LOG_ERROR("listen: %s\n", strerror(errno));
        close(*sck);
        return HTTP_ERR;
    }

    return HTTP_OK;
}

int32_t
socket_connect(socket_t *sck, const char *ip_addr, int32_t portno) {
    struct sockaddr_in addr;

    if ((*sck = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        LOG_ERROR("socket: %s\n", strerror(errno));
        return HTTP_ERR;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(portno);
    addr.sin_addr.s_addr = ((ip_addr) ? inet_addr(ip_addr) : INADDR_ANY);

    if (connect(*sck, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        LOG_ERROR("connect: %s\n", strerror(errno));
        close(*sck);
        return HTTP_ERR;
    }

    return HTTP_OK;
}

int32_t
socket_close(socket_t *sck) {
    if (close(*sck) == -1) {
        LOG_ERROR("close: %s\n", strerror(errno));
        return HTTP_ERR;
    }

    return HTTP_OK;
}

int32_t
socket_accept(socket_t *sck, int32_t *fd) {
    if ((*fd = accept(*sck, NULL, NULL)) == -1) {
        LOG_INFO("accept: %s\n", strerror(errno));
        return HTTP_ERR;
    }

    return HTTP_OK;
}

int32_t
socket_recv(socket_t *sck, void *data, int32_t len) {
    int32_t nread;

    memset(data, 0, len);
    if ((nread = recv(*sck, data, len, 0)) == -1) {
        LOG_INFO("recv: %s\n", strerror(errno));
    }

    return nread;
}

int32_t
socket_send(socket_t *sck, const void *data, int32_t len) {
    if (send(*sck, data, len, 0) == -1) {
        LOG_INFO("send: %s\n", strerror(errno));
        return HTTP_ERR;
    }

    return HTTP_OK;
}
