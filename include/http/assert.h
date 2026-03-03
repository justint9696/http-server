#ifndef _HTTP_ASSERT_H
#define _HTTP_ASSERT_H

#include "http/logger.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#ifndef ASSERT
#define ASSERT(_e) do { \
    if (!(_e)) { \
        LOG_PANIC("Assertion failure: `%s`\n", #_e); \
    } \
} while (0)
#endif // ASSERT

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _HTTP_ASSERT_H
