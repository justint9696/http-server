#ifndef _UNIT_TESTER_H
#define _UNIT_TESTER_H

#include "http/client.h"
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
#define LOG_FILE        "logs/unit_tests.log"


typedef struct {
    int32_t             rdy;
    pthread_t           th;
    pthread_mutex_t     mtx;
    pthread_cond_t      cd;
    server_t            sv;
    client_t            cl;
} state_t;

int
log_init(const char *fname, int32_t stdout_level, int32_t fp_level) {
    if (!logger_set_file(fname, HTTP_TRUE)) {
        fprintf(stderr, "Error: Failed to create log file `%s`\n", fname);
        return UT_ERR;
    }

    logger_set_level(stdout_level, fp_level);

    return UT_OK;
}

#endif // _UNIT_TESTER_H
