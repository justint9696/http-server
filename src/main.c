#include <getopt.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "logger.h"
#include "server.h"
#include "types.h"

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
    int32_t     clientfd;
    int32_t     port;
    server_t    server;
    packet_t    pkt;
} state_t;

static const char *errstr =
"Usage: %s --port p --module m [OPTIONS]...\n";

static int
print_usage(state_t *state);

static int
parse_options(state_t *state, int argc, char **argv);

int
main(int argc, char **argv) {
    int ret = OK;
    state_t state;

    memset(&state, 0, sizeof(state));
    strcpy(state.pname, basename(*argv));

    if (argc < 2) {
        fprintf(stderr, errstr, state.pname);
        goto err;
    }
    
    if (!(ret = parse_options(&state, argc, argv))) {
        ret = ERR;
        goto err;
    }

    if (!(ret = logger_set_file(LOG_FILE, HTTP_FALSE))) {
        fprintf(stderr, "Failed to initialize logger\n");
        goto err;
    }

    logger_set_level(LL_WARN, LL_DEBUG);
    if (!(ret = server_init(&state.server, state.port))) {
        LOG_ERROR("Failed to initialize server\n");
        ret = ERR;
        goto err;
    }

    while (ret) {
        if (!(ret = server_accept(&state.server, &state.clientfd))) {
            LOG_ERROR("Failed to connect to incoming client\n");
        } 

        if (ret &&
                !(ret = server_recv(&state.server, state.clientfd, &state.pkt))) {
            LOG_ERROR("Failed to receive data from client.\n");
        }

        LOG_INFO("Client says:\n%s\n", state.pkt.data);
    }

err:
    return ((ret) ? SUCCESS : FAILURE);
}

static int
print_usage(state_t *state) {
    fprintf(stderr, errstr, state->pname);
    fprintf(stderr, "\nOptions:\n");
    fprintf(stderr, "   -p, --port=PORT          server port\n");
    fprintf(stderr, "   -m, --module=DIRECTORY   web module location\n");
    fprintf(stderr, "   -v, --verbose            enable verbose logging\n");
    fprintf(stderr, "   -h, --help               display help message\n");
    return OK;
}

static int
parse_options(state_t *state, int argc, char **argv) {
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
                state->verbose = HTTP_TRUE;
                break;
            case 'p':
                state->port = atoi(optarg);
                break;
            case 'm':
                state->module = optarg;
                break;
            case 'h':
                print_usage(state);
                ret = ERR;
                break;
            case '?': 
                fprintf(stderr, "Unknown parameter `%c`\n", opt);
                break;
        }
    }

    if (ret && !state->port) {
        fprintf(stderr, "Missing required parameter `port`\n");
        ret = ERR;
    }

    if (ret && !state->module) {
        fprintf(stderr, "Missing required parameter `module`\n");
        ret = ERR;
    }

    return ret;
}
