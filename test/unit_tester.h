#ifndef _UNIT_TESTER_H
#define _UNIT_TESTER_H

#include "client.h"
#include "server.h"

#include <pthread.h>

#define UT_OK         1
#define UT_ERR        0

#define UT_SUCCESS    0
#define UT_FAILURE    1

typedef struct {
    char                *fname;
    pthread_t           th;
    pthread_mutex_t     mtx;
    server_t            sv;
    client_t            cl;
} state_t;

#endif // _UNIT_TESTER_H
