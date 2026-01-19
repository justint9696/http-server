#include "unit_tester.h"

#include <libgen.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define SERVER_PORT     3000
#define LOG_FILE        "logs/unit_tests.log"

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
    FILE *fp;
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
    if (!(fp = fopen(LOG_FILE, "w+"))) {
        perror("fopen");
        return UT_FAILURE;
    }

    // initialize server
    if (!(ret = server_init(&state.sv, SERVER_PORT))) {
        return UT_FAILURE;
    }

    pthread_create(&state.th, NULL, server_thread, (void *)&state);

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
    fclose(fp);

    return UT_SUCCESS;
}

static int
run_test(const char *pname, state_t *state) {
    int wstatus;
    int ret = UT_SUCCESS;
    pid_t pid;

    switch ((pid = fork())) {
        case -1:
            perror("fork");
            return -1;
        case 0:
            if (execl(pname, pname, LOG_FILE, NULL) == -1) {
                perror("execl");
                return UT_FAILURE;
            }
            break;
        default:
            if (waitpid(pid, &wstatus, 0) == -1) {
                perror("waitpid");
                return UT_FAILURE;
            }

            if (WIFEXITED(wstatus)) {
                ret = WEXITSTATUS(wstatus);
            } else if (WIFSIGNALED(wstatus)) {
                ret = WTERMSIG(wstatus);
            }

            break;
    }
    
    return ret;
}

static void *
server_thread(void *args) {
    int fd;
    packet_t pkt;
    state_t *state;

    state = (state_t *)args;
    if (!server_listen(&state->sv, &fd, &pkt)) {
        fprintf(stderr, "Failed to connect to incoming connection\n");
        return NULL;
    }

    return NULL;
}
