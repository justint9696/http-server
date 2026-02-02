#include "unit_tester.h"

#include <string.h>

int
main(int argc, char **argv) {
    int ret;
    client_t cl;
    packet_t pkt;

    if (!(ret = log_init(LOG_FILE, LL_NONE, LL_DEBUG))) {
        fprintf(stderr, "Error: Failed to create log file\n");
        return UT_FAILURE;
    }

    if (ret && !(ret = client_init(&cl, SERVER_PORT))) {
        LOG_ERROR("Failed to initialize client\n");
    }

    LOG_INFO("Client is ready.\n");

    memset(&pkt, 0, sizeof(pkt));
    strcpy((char *)pkt.data, "Hello from client");
    pkt.len = strlen((char *)pkt.data);

    LOG_INFO("Sending `%s` to server.\n", (char *)pkt.data);

    if (ret && !(ret = client_send(&cl, &pkt))) {
        LOG_ERROR("Failed to send packet\n");
    }

    LOG_INFO("%d bytes sent.\n", pkt.len);
    client_destroy(&cl);

    logger_close_file();

    return ((ret) ? UT_SUCCESS : UT_FAILURE);
}
