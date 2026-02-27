#ifndef _HTTP_LOGGER_H
#define _HTTP_LOGGER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#ifdef _DEBUG
#define LOG_TRACE(_fmt, ...) \
    _log_intern(LL_TRACE, _fmt, ##__VA_ARGS__)

#define LOG_DEBUG(_fmt, ...) \
    _log_intern(LL_DEBUG, _fmt, ##__VA_ARGS__)
#else
#define LOG_TRACE(_fmt, ...)

#define LOG_DEBUG(_fmt, ...)
#endif // _DEBUG

#define LOG_INFO(_fmt, ...) \
    _log_intern(LL_INFO, _fmt, ##__VA_ARGS__)

#define LOG_WARN(_fmt, ...) \
    _log_intern(LL_WARN, _fmt, ##__VA_ARGS__)

#define LOG_ERROR(_fmt, ...) \
    _log_intern(LL_ERROR, _fmt, ##__VA_ARGS__)

#define LOG_FATAL(_fmt, ...) \
    _log_intern(LL_FATAL, _fmt, ##__VA_ARGS__)

#define LOG_PANIC(_fmt, ...) \
    _log_intern(LL_PANIC, _fmt, ##__VA_ARGS__)

enum _log_level {
    LL_TRACE = 0,
    LL_DEBUG,
    LL_INFO,
    LL_WARN,
    LL_ERROR,
    LL_FATAL,
    LL_PANIC,
    LL_NONE,
};

int32_t
_log_intern(int32_t level, const char *fmt, ...);

int32_t
logger_set_file(const char *fname, int32_t append);

int32_t
logger_set_level(int32_t stdout_level, int32_t file_level);

int32_t
logger_close_file(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _HTTP_LOGGER_H
