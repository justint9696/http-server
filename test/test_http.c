#include "http/http.h"
#include "http/logger.h"
#include "unit_tester.h"

#include <string.h>

#define RQST_TBL_LEN 4

static struct {
    char    *message;
    int     expect;
} RQST_TBL[RQST_TBL_LEN] = {
    {
        "GET / HTTP/1.1\r\n"
        "Host: localhost:8000\r\n"
        "Accept: */*\r\n",
        HTTP_RES_OK
    },

    {
        "POST /users HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Type: application/x-www-form-urlencoded\r\n"
        "Content-Length: 49\r\n"
        "\r\n"
        "name=FirstName+LastName&email=bsmth%40example.com",
        HTTP_RES_NOT_FOUND
    },


    {
        "GE T / HTTP/1.1\r\n",
        HTTP_RES_BAD_REQUEST
    },

    {
        "GET /\r\n",
        HTTP_RES_BAD_REQUEST
    }
};

int
main(int argc, char **argv) {
    int ret = UT_OK;
    int rc;
    int32_t i;
    const __typeof__(*RQST_TBL) *rqst = NULL;
    char data[2048];
    client_t cl;
    http_t http;

    if (!log_init()) {
        fprintf(stderr, "Error: Failed to create log file\n");
        return UT_FAILURE;
    }

    memset(&cl, 0, sizeof(cl));
    memset(&http, 0, sizeof(http));
    if (!(rc = client_init(&cl, SERVER_PORT))) {
        LOG_ERROR("Failed to initialize client\n");
        ret = UT_ERR;
    }

    for (i = 0; i < RQST_TBL_LEN; i++) {
        rqst = &RQST_TBL[i];

        LOG_DEBUG("Sending HTTP request %d\n", (i+1));
        LOG_TRACE("%s\n", rqst->message);
        if (rc && !(rc = client_send(
                        &cl, rqst->message, strlen(rqst->message)))) {
            LOG_ERROR("Failed to send packet\n");
            ret = UT_ERR;
            break;
        }

        LOG_DEBUG("Waiting for server response\n");
        if (rc && (rc = client_recv(&cl, data, sizeof(data))) == -1) {
            LOG_ERROR("Client failed to receive server response\n");
            ret = UT_ERR;
            break;
        }

        LOG_DEBUG("Client received %d bytes from server\n", rc);
        LOG_TRACE("%s\n", data);
        if (!(rc = http_parse_message(&http, data, rc))) {
            LOG_ERROR("Failed to parse HTTP message\n");
            ret = UT_ERR;
            break;
        } else {
            LOG_TRACE("Server response:\n");
            LOG_TRACE("HTTP type: %d\n", http.type);
            LOG_TRACE("HTTP status code: %d\n", http.rc);
            LOG_TRACE("HTTP method: %d\n", http.method);
            LOG_TRACE("HTTP body: %.*s\n", http.body.len, http.body.str);
        }

        if (http.rc != rqst->expect) {
            LOG_ERROR("Return code `%d` does not match expected result `%d`\n",
                    http.rc, rqst->expect);
            ret = UT_ERR;
            break;
        }
    }

    client_destroy(&cl);
    logger_close_file();

    return ((ret) ? UT_SUCCESS : UT_FAILURE);
}
