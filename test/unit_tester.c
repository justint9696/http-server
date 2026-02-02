#include "unit_tester.h"

#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

const char *errstr =
"Usage: %s [TEST]...\n";

static int
run_test(const char *pname, state_t *state);

static void *
server_thread(void *args);

int
main(int argc, char **argv) {
    int i;
    int ret;
    state_t state;
    char msg[128];
    time_t start;
    time_t end;

    if (argc < 2) {
        fprintf(stderr, errstr, basename(*argv));
        return UT_SUCCESS;
    }

    memset(&state, 0, sizeof(state_t));

    // create and truncate the log file
    if (!(ret = logger_set_file(LOG_FILE, HTTP_FALSE))) {
        return UT_FAILURE;
    }

    logger_set_level(LL_NONE, LL_DEBUG);

    pthread_create(&state.th, NULL, server_thread, (void *)&state);

    // wait for server initialization
    while (!state.rdy)
        pthread_cond_wait(&state.cd, &state.mtx);

    printf("\nUnit test results...\n");
    time(&start);
    {
        for (i = 1; i < argc; i++) {
            ret = run_test(argv[i], &state);
            snprintf(msg, sizeof(msg), "[%d/%d] Running %s",
                    i, argc - 1, basename(argv[i]));
            printf("   %-32s [%s]\n", msg, ((ret == UT_SUCCESS) ? "OK" : "Fail"));
        }
    }
    time(&end);
    printf("\nDone (%.2lfs)\n", (end - start) / 1e+6);

    // kill the server thread
    pthread_cancel(state.th);

    // cleanup
    server_destroy(&state.sv);

    return UT_SUCCESS;
}

static int
run_test(const char *pname, state_t *state) {
    int wstatus;
    pid_t pid;

    switch ((pid = fork())) {
        case -1:
            perror("fork");
            return UT_FAILURE;
        case 0:
            execl(pname, pname, NULL);
            perror("execl");
            exit(UT_FAILURE);
        default:
            if (waitpid(pid, &wstatus, 0) == -1) {
                perror("waitpid");
                return UT_FAILURE;
            }

            if (WIFEXITED(wstatus))
                return WEXITSTATUS(wstatus);
            
            if (WIFSIGNALED(wstatus))
                return 128 + WTERMSIG(wstatus);

            break;
    }
    
    return UT_FAILURE;
}

static void *
server_thread(void *args) {
    int fd;
    packet_t pkt;
    state_t *state;

    state = (state_t *)args;
    pthread_mutex_lock(&state->mtx);

    (void)server_init(&state->sv, SERVER_PORT);
    state->rdy = HTTP_TRUE;

    pthread_cond_signal(&state->cd);
    pthread_mutex_unlock(&state->mtx);

    while (1) {
        LOG_INFO("Server is ready.\n");
        if (!server_accept(&state->sv, &fd)) {
            LOG_ERROR("Failed to connect to incoming connection\n");
            continue;
        }

        if (!server_recv(&state->sv, fd, &pkt)) {
            LOG_ERROR("Hey there\n");
            continue;
        }

        LOG_INFO("Client says: `%s`\n", (char *)pkt.data);
    }

    return NULL;
}
