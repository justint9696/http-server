#include "unit_tester.h"

#include <string.h>

int
main(int argc, char **argv) {
    int ret = UT_OK;
    int rc;
    client_t cl;
    uint8_t data[1024];
    int32_t len;

    if (!(rc = log_init())) {
        fprintf(stderr, "Error: Failed to create log file\n");
        return UT_FAILURE;
    }

    if (rc && !(rc = client_init(&cl, SERVER_PORT))) {
        LOG_ERROR("Failed to initialize client\n");
        ret = UT_ERR;
    } else {
        LOG_INFO("Client is ready\n");
    }

    memset(data, 0, sizeof(data));
    strcpy((char *)data, "Hello from client");
    len = strlen((char *)data);

    if (rc) {
        LOG_DEBUG("Sending `%s` to server\n", (char *)data);
        if (!(rc = client_send(&cl, data, len))) {
            LOG_WARN("Failed to send packet\n");
            ret = UT_ERR;
        }
    }

    client_destroy(&cl);
    logger_close_file();

    return ((ret) ? UT_SUCCESS : UT_FAILURE);
}
