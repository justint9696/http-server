#include "logger.h"
#include "types.h"

#include <fcntl.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

static struct {
    int             fd;
    pthread_mutex_t mtx;
    int32_t         stdout_level;
    int32_t         file_level;
} g_logger = { -1, PTHREAD_MUTEX_INITIALIZER, LL_WARN, LL_INFO };

int32_t
_log_intern(int32_t level, const char *fmt, ...) {
    int ret = HTTP_OK;
    size_t len;
    time_t ts;
    va_list args;
    char timestr[64];
    char text[1024];
    struct timeval tv;

    pthread_mutex_lock(&g_logger.mtx);

    time(&ts);
    gettimeofday(&tv, NULL);
    strftime(timestr, sizeof(timestr), "%M:%S", localtime(&ts));

    snprintf(text, sizeof(text), "[%s:%03ld] ", timestr, (tv.tv_usec / 1000));
    len = strlen(text);

    va_start(args, fmt);
    vsnprintf(&text[len], sizeof(text) - len, fmt, args);
    va_end(args);

    if (level >= g_logger.file_level && g_logger.fd != -1) {
        if (write(g_logger.fd, (void *)text, strlen(text)) == -1) {
            perror("write");
            ret = HTTP_ERR;
        }
    }

    if (level >= g_logger.stdout_level) {
        fprintf(stdout, "%s", text);
        fflush(stdout);
    }

    pthread_mutex_unlock(&g_logger.mtx);

    switch (level) {
        case LL_FATAL:
#ifdef _DEBUG
            abort();
#else
            exit(HTTP_FAILURE);
#endif // _DEBUG
            break;
        default:
            return ret;
    }
}

int32_t
logger_set_file(const char *fname, int32_t append) {
    int flags = (O_RDWR | O_APPEND | O_CREAT);
    flags |= ((append) ? 0 : O_TRUNC);

    if ((g_logger.fd = open(fname, flags, 0644)) == -1) {
        perror("open");
        return HTTP_ERR;
    }

    return HTTP_OK;
}

int32_t
logger_set_level(int32_t stdout_level, int32_t file_level) {
    g_logger.stdout_level = stdout_level;
    g_logger.file_level = file_level;
    return HTTP_OK;
}

int32_t
logger_close_file(void) {
    if (g_logger.fd != -1) {
        close(g_logger.fd);
        g_logger.fd = -1;
    }

    return HTTP_OK;
}
