#ifndef _ASSERT_H
#define _ASSERT_H

#include "http/logger.h"

#ifndef ASSERT
#define ASSERT(_e) do { \
    if (!(_e)) { \
        LOG_PANIC("Assertion failure: `%s`\n", #_e); \
    } \
} while (0)
#endif // ASSERT

#endif // _ASSERT_H
