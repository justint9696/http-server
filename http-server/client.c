#include "http/client.h"
#include "http/logger.h"
#include "http/types.h"

#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int32_t
client_init(client_t *cl, int32_t portno) {
    struct sockaddr_in addr;

    memset(cl, 0, sizeof(client_t));
    if ((cl->sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        LOG_ERROR("socket: %s\n", strerror(errno));
        return HTTP_ERR;
    }

    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(portno);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (connect(cl->sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        LOG_ERROR("connect: %s\n", strerror(errno));
        close(cl->sockfd);
        return HTTP_ERR;
    }

    return HTTP_OK;
}

int32_t
client_destroy(client_t *cl) {
    shutdown(cl->sockfd, SHUT_RDWR);
    close(cl->sockfd);
    return HTTP_OK;
}

int32_t
client_send(client_t *cl, const void *data, int32_t len) {
    LOG_DEBUG("Sending %d bytes to server\n");
    if (send(cl->sockfd, data, len, 0) == -1) {
        LOG_ERROR("send: %s\n", strerror(errno));
        return HTTP_ERR;
    }
    return HTTP_OK;
}

int32_t
client_recv(client_t *cl, void *data, int32_t *len) {
    if (recv(cl->sockfd, data, *len, 0) == -1) {
        LOG_ERROR("recv: %s\n", strerror(errno));
        return HTTP_ERR;
    }

    *len = strlen((char *)data);

    return HTTP_OK;
}
