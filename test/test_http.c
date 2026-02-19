#include "http/http.h"
#include "http/logger.h"
#include "unit_tester.h"

#include <string.h>

#define RQST_TBL_LEN 2

const char *RQST_TBL[RQST_TBL_LEN] = {
    "GET / HTTP/1.1\n"
    "Host: localhost:8000\n"
    "Accept: */*\n",

    "POST /users HTTP/1.1\n"
    "Host: example.com\n"
    "Content-Type: application/x-www-form-urlencoded\n"
    "Content-Length: 49\n"
    "\n"
    "name=FirstName+LastName&email=bsmth%40example.com",
};

int
main(int argc, char **argv) {
    int ret;
    client_t cl;
    int32_t i;
    const char *request = NULL;

    if (!log_init()) {
        fprintf(stderr, "Error: Failed to create log file\n");
        return UT_FAILURE;
    }

    memset(&cl, 0, sizeof(cl));
    if (!(ret = client_init(&cl, SERVER_PORT))) {
        LOG_ERROR("Failed to initialize client\n");
    }

    for (i = 0; i < RQST_TBL_LEN; i++) {
        request = RQST_TBL[i];

        LOG_INFO("Sending HTTP request %d\n", (i+1));
        if (ret && !(ret = client_send(&cl, request, strlen(request)))) {
            LOG_ERROR("Failed to send packet\n");
        }
    }

    client_destroy(&cl);
    logger_close_file();

    return 0;
}
