#ifndef _UNIT_TESTER_H
#define _UNIT_TESTER_H

#include "http/client.h"
#include "http/http.h"
#include "http/logger.h"
#include "http/server.h"
#include "http/types.h"

#include <pthread.h>
#include <stdio.h>

#define UT_OK           1
#define UT_ERR          0

#define UT_SUCCESS      0
#define UT_FAILURE      1

#define SERVER_PORT     3000
#define SERVER_ROOT     "test/res/public"
#define LOG_FILE        "logs/tests.log"
#define LOG_LEVEL       LL_TRACE


typedef struct _ctx {
    int32_t             rdy;
    pthread_t           th;
    pthread_mutex_t     mtx;
    pthread_cond_t      cd;
    server_t            sv;
    client_t            cl;
    http_t              http;
} ctx_t;

int
log_init() {
    if (!logger_set_file(LOG_FILE, HTTP_TRUE)) {
        fprintf(stderr, "Error: Failed to create log file `%s`\n", LOG_FILE);
        return UT_ERR;
    }

    logger_set_level(LL_NONE, LL_DEBUG);

    return UT_OK;
}

#endif // _UNIT_TESTER_H
