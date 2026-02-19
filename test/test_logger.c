#include "unit_tester.h"

#include <stdio.h>

int main(int argc, char **argv) {
    if (!log_init()) {
        fprintf(stderr, "Error: Failed to create log file\n");
        return UT_FAILURE;
    }

    LOG_DEBUG("%s", "This is a debug message\n");
    LOG_INFO("%s", "This is an info message\n");
    LOG_WARN("%s", "This is a warning message\n");
    LOG_ERROR("%s", "This is an error message\n");

    logger_close_file();

    return UT_SUCCESS;
}
