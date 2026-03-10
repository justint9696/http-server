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
run_test(const char *pname, ctx_t *ctx);

static void *
server_thread(void *args);

int
main(int argc, char **argv) {
    int i;
    int ret;
    ctx_t ctx;
    char msg[128];
    time_t start;
    time_t end;

    if (argc < 2) {
        fprintf(stderr, errstr, basename(*argv));
        return UT_SUCCESS;
    }

    memset(&ctx, 0, sizeof(ctx_t));

    // create and truncate the log file
    if (!(ret = logger_set_file(LOG_FILE, HTTP_FALSE))) {
        return UT_FAILURE;
    }

    logger_set_level(LL_NONE, LL_DEBUG);

    pthread_create(&ctx.th, NULL, server_thread, (void *)&ctx);

    // wait for server initialization
    while (!ctx.rdy) {
        pthread_cond_wait(&ctx.cd, &ctx.mtx);
    }

    printf("\nUnit test results...\n");
    time(&start);
    {
        for (i = 1; i < argc; i++) {
            snprintf(msg, sizeof(msg), "[%d/%d] Running %s",
                    i, argc - 1, basename(argv[i]));
            LOG_INFO("%s\n", msg);

            ret = run_test(argv[i], &ctx);
            printf("   %-32s [%s]\n",
                    msg, ((ret == UT_SUCCESS) ? "OK" : "Fail"));
        }
    }
    time(&end);
    printf("\nDone (%.3lfs)\n", (end - start) / 1000.0f);

    // kill the server thread
    pthread_cancel(ctx.th);

    // cleanup
    server_destroy(&ctx.sv);

    return UT_SUCCESS;
}

static int
run_test(const char *pname, ctx_t *ctx) {
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
    int retry;
    uint8_t data[1024];
    int32_t state;
    int32_t ret = 0;
    ctx_t *ctx = NULL;
    http_t rqst;
    http_t rspn;

    ctx = (ctx_t *)args;
    pthread_mutex_lock(&ctx->mtx);

    (void)server_init(&ctx->sv, SERVER_PORT, SERVER_ROOT);
    ctx->rdy = HTTP_TRUE;

    pthread_cond_signal(&ctx->cd);
    pthread_mutex_unlock(&ctx->mtx);

    enum { SV_QUIT = 0, SV_DISCONNECT, SV_IDLE, SV_ACTIVE };
    state = SV_IDLE;

    while (1) {
        switch (state) {
            case SV_DISCONNECT:
                LOG_INFO("Client %d disconnected\n", fd);
                close(fd);
                state = SV_IDLE;
                break;
            case SV_IDLE:
                LOG_INFO("Server is ready\n");
                if (!server_accept(&ctx->sv, &fd)) {
                    LOG_WARN("Failed to connect to client\n");
                    continue;
                }

                LOG_INFO("Client %d connected\n", fd);

                retry = 0;
                state = SV_ACTIVE;
                break;
            case SV_ACTIVE:
                if ((ret = server_recv(
                                &ctx->sv, fd, data, sizeof(data))) == -1) {
                    LOG_INFO("Server failed to receive message\n");
                    if (++retry > 3) {
                        LOG_INFO("Server exhausted retries\n");
                        state = SV_DISCONNECT;
                    }
                    break;
                } else if (ret == 0) {
                    LOG_DEBUG("Received 0 bytes, closing connection\n");
                    state = SV_DISCONNECT;
                    break;
                }

                LOG_DEBUG("Server received %d bytes\n", ret);
                if (!(ret = http_parse_message(&rqst, (char *)data, ret))) {
                    LOG_WARN("Failed to parse client data\n");
                }

                if (!http_fmt_response(&rqst, &rspn, ctx->sv.dirname)) {
                    LOG_ERROR("Failed to format response\n");
                    break;
                }

                if (!server_send(&ctx->sv, fd, (void *)rspn.buf, rspn.len)) {
                    LOG_ERROR("Failed to send response\n");
                    break;
                }

                LOG_DEBUG("Sent %d bytes to client\n", rspn.len);
                break;
            default:
                return NULL;
        }
    }

    return NULL;
}
