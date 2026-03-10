#include "http/file_io.h"
#include "unit_tester.h"

int main(int argc, char **argv) {
    int ret = UT_OK;
    int nread;
    int offset;
    int size;
    int fd;
    unsigned char buf[2];

    if (!log_init()) {
        fprintf(stderr, "Error: Failed to create log file\n");
        return UT_FAILURE;
    }

    if ((fd = file_open("test/res/test.txt")) == -1) {
        LOG_ERROR("Failed to open file\n");
        return UT_FAILURE;
    }

    if (!(size = file_size(fd))) {
        LOG_ERROR("File is empty\n");
        ret = UT_ERR;
        goto err;
    }

    LOG_DEBUG("Streaming %d bytes\n", size);

    offset = 0;
    while ((nread = file_stream(fd, offset, buf, sizeof(buf))) > 0) {
        LOG_TRACE("Streamed %d bytes: %.*s\n", nread, nread, buf);
        offset += nread;
    }

    if (nread < 0) {
        ret = UT_ERR;
        goto err;
    }

err:
    file_close(fd);
    logger_close_file();

    return ((ret) ? UT_SUCCESS : UT_FAILURE);
}
