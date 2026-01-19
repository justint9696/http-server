#include "logger.h"
#include "types.h"
#include "unit_tester.h"

#include <stdio.h>

int main(int argc, char **argv) {
    char *fname = NULL;

    if (argc != 2)
        return UT_FAILURE;

    fname = argv[1];
    if (!logger_set_file(fname, HTTP_TRUE)) {
        fprintf(stderr, "Error: Failed to create log file `%s`\n", fname);
        return UT_FAILURE;
    }

    (void)logger_set_level(LL_NONE, LL_DEBUG);

    LOG_DEBUG("%s", "This is a debug message\n");
    LOG_ERROR("%s", "This is an error message\n");
    LOG_FATAL("%s", "This is a fatal message\n");

    logger_close_fp();

    return UT_SUCCESS;
}
