#include "client.h"
#include "logger.h"
#include "types.h"

#include <errno.h>
#include <netinet/in.h>
#include <stdlib.h>
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

    addr.sin_family = AF_INET;
    addr.sin_port = htons(portno);
    addr.sin_addr.s_addr = INADDR_ANY;

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
client_send(client_t *cl, packet_t *pkt) {
    return HTTP_OK;
}

int32_t
client_recv(client_t *cl, packet_t *pkt) {
    if (recv(cl->sockfd, (void *)pkt->data, sizeof(pkt->data), 0) == -1) {
        LOG_ERROR("recv: %s\n", strerror(errno));
        return HTTP_ERR;
    }

    pkt->len = strlen((char *)pkt->data);

    return HTTP_OK;
}
