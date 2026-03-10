#include "unit_tester.h"

static int32_t
strntoi(const char *s, int32_t len) {
    int ret = 0;
    while (s && *s != '\0' && --len >= 0) {
        ret = ((ret * 10) + (*s++ - '0'));
    }
    return ret;
}

int
main(int argc, char **argv) {
    int i;
    int res;
    int ret;

    if (!log_init()) {
        fprintf(stderr, "Error: Failed to create log file\n");
        return UT_FAILURE;
    }

    static const struct {
        char    *ascii;
        int     len;
        int     expect;
    } TEST_XREF[] = {
        { "128",        3,  128 },
        { "256",        3,  256 },
        { "1011011",    7,  1011011 },
        { "0000001",    7,  1 },
    };

    static int num_test = (sizeof(TEST_XREF) / sizeof(TEST_XREF[0]));
    const __typeof__(TEST_XREF[0]) *xref = NULL;

    ret = UT_OK;
    for (i = 0; i < num_test; i++) {
        xref = &TEST_XREF[i];
        if ((res = strntoi(xref->ascii, xref->len)) != xref->expect) {
            LOG_ERROR("Result %d does not match expected %d\n",
                      res, xref->expect);
            ret = UT_ERR;
        }
    }

    return ((ret) ? UT_SUCCESS : UT_FAILURE);
}
