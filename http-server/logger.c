#include "logger.h"
#include "types.h"

#include <stdarg.h>
#include <string.h>
#include <time.h>

static struct {
    FILE        *fp;
    int32_t     stdout_level;
    int32_t     fp_level;
} g_logger = { NULL, LL_WARN, LL_INFO };

int32_t
_log_intern(int32_t level, const char *fmt, ...) {
    time_t ts;
    size_t len;
    va_list args;
    char text[1024];

    time(&ts);
    strftime(text, sizeof(text), "[%H:%M:%S] ", localtime(&ts));
    len = strlen(text);

    va_start(args, fmt);
    vsnprintf(text + len, sizeof(text) - len, fmt, args);
    va_end(args);

    if (level >= g_logger.fp_level) {
        fprintf(g_logger.fp, "%s", text);
        fflush(g_logger.fp);
    }

    if (level >= g_logger.stdout_level) {
        fprintf(stdout, "%s", text);
        fflush(stdout);
    }

    return HTTP_OK;
}

int32_t
logger_set_file(const char *fname, int32_t append) {
    if (!(g_logger.fp = fopen(fname, ((append) ? "a+" : "w+")))) {
        perror("fopen");
        return HTTP_ERR;
    }

    return HTTP_OK;
}

int32_t
logger_set_level(int32_t stdout_level, int32_t fp_level) {
    g_logger.stdout_level = stdout_level;
    g_logger.fp_level = fp_level;
    return HTTP_OK;
}

int32_t
logger_close_fp(void) {
    if (g_logger.fp) {
        fclose(g_logger.fp);
        return HTTP_OK;
    }

    return HTTP_ERR;
}
