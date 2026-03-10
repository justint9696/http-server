#include <errno.h>
#include <getopt.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "http/logger.h"
#include "http/http.h"
#include "http/server.h"
#include "http/types.h"

#define OK      1
#define ERR     0
#define SUCCESS 0
#define FAILURE 1

#define LOG_FILE "logs/server.log"

typedef struct {
    FILE        *fp;
    char        pname[32];
    char        *module;
    int32_t     verbose;
    int32_t     port;
    server_t    server;
} ctx_t;

static const char *errstr =
"Usage: %s --port p --module m [OPTIONS]...\n";

static int
print_usage(ctx_t *ctx);

static int
parse_options(ctx_t *ctx, int argc, char **argv);

static int
server_run(ctx_t *ctx);

static int
handle_client_connection(ctx_t *ctx, int clientfd);

int
main(int argc, char **argv) {
    int ret = OK;
    ctx_t ctx;

    memset(&ctx, 0, sizeof(ctx));
    strcpy(ctx.pname, basename(*argv));

    if (argc < 2) {
        fprintf(stderr, errstr, ctx.pname);
        goto err;
    }
    
    if (!(ret = parse_options(&ctx, argc, argv))) {
        ret = ERR;
        goto err;
    }

    if (!(ret = logger_set_file(LOG_FILE, HTTP_FALSE))) {
        fprintf(stderr, "Failed to initialize logger\n");
        goto err;
    }

    logger_set_level(LL_WARN, ((ctx.verbose == HTTP_TRUE) ? LL_TRACE : LL_INFO));
    if (!(ret = server_init(&ctx.server, ctx.port, ctx.module))) {
        LOG_ERROR("Failed to initialize server\n");
        ret = ERR;
        goto err;
    }

    log_write("Server started on port %d\n", ctx.port);
    if (!(ret = server_run(&ctx))) {
        LOG_INFO("Server terminated\n");
        goto err;
    }

err:
    return ((ret) ? SUCCESS : FAILURE);
}

static int
print_usage(ctx_t *ctx) {
    fprintf(stderr, errstr, ctx->pname);
    fprintf(stderr, "\nOptions:\n");
    fprintf(stderr, "   -p, --port=PORT          server port\n");
    fprintf(stderr, "   -m, --module=DIRECTORY   web module location\n");
    fprintf(stderr, "   -v, --verbose            enable verbose logging\n");
    fprintf(stderr, "   -h, --help               display help message\n");
    return OK;
}

static int
parse_options(ctx_t *ctx, int argc, char **argv) {
    int ret = OK;
    int opt;

    static struct option long_options[] = {
        { "port", required_argument, 0, 'p' },
        { "module", required_argument, 0, 'm' },
        { "verbose", no_argument, 0, 'v' },
        { "help", no_argument, 0, 'h' },
        { 0, 0, 0, 0 }
    };

    while (ret && (opt = getopt_long(
                    argc, argv, ":p:m:vh", long_options, NULL)) != -1) {
        switch (opt) {
            case 'v':
                ctx->verbose = HTTP_TRUE;
                break;
            case 'p':
                ctx->port = atoi(optarg);
                break;
            case 'm':
                ctx->module = optarg;
                break;
            case 'h':
                print_usage(ctx);
                ret = ERR;
                break;
            case '?': 
                fprintf(stderr, "Unknown parameter `%c`\n", opt);
                break;
        }
    }

    if (ret && !ctx->port) {
        fprintf(stderr, "Missing required parameter `port`\n");
        ret = ERR;
    }

    if (ret && !ctx->module) {
        fprintf(stderr, "Missing required parameter `module`\n");
        ret = ERR;
    }

    return ret;
}

static int
server_run(ctx_t *ctx) {
    int fd = -1;
    pid_t pid;

    while (1) {
        if (!server_accept(&ctx->server, &fd)) {
            LOG_WARN("Failed to connect to incoming client\n");
            continue;
        }

        switch ((pid = fork())) {
            case -1:
                LOG_ERROR("fork: %s\n", strerror(errno));
                break;
            case 0:
                exit(handle_client_connection(ctx, fd));
                break;
            default:
                LOG_INFO("Spawned child process %d\n", pid);
                break;
        }
    }

    return OK;
}

static int
handle_client_connection(ctx_t *ctx, int fd) {
    int nread = 0;
    int rc;
    http_t rqst;
    http_t rspn;
    unsigned char data[8192];

    while (1) {
        memset(data, '\0', sizeof(data));
        if ((nread = server_recv(
                        &ctx->server, fd, data, sizeof(data))) == -1) {
            LOG_WARN("Failed to receive message\n");
            break;
        } else if (nread == 0) {
            LOG_DEBUG("Received 0 bytes, closing connection\n");
            break;
        }

        LOG_DEBUG("Received %d bytes from client %d\n", nread, fd);
        if (!(rc = http_parse_message(&rqst, (char *)data, nread))) {
            LOG_WARN("Failed to parse client request\n");
        }

        if (!http_fmt_response(&rqst, &rspn, ctx->server.dirname)) {
            LOG_WARN("Failed for format client response\n");
            break;
        }

        LOG_DEBUG("Sending %d bytes to client %d\n", rqst.len, fd);
        if (!server_send(
                    &ctx->server,
                    fd,
                    (void *)rqst.buf,
                    rqst.len)) {
            LOG_WARN("Failed to send client response\n");
            break;
        }
        LOG_DEBUG("Success\n");
    }

    LOG_DEBUG("Client %d disconnected\n", fd);
    close(fd);

    return SUCCESS;
}
