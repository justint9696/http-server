#include "http/file_io.h"
#include "http/logger.h"
#include "http/types.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int32_t
file_open(const char *fname) {
    int fd;
    int flags;
    struct stat st;

    flags = O_RDONLY;
    if ((fd = open(fname, flags)) == -1) {
        LOG_INFO("open: %s\n", strerror(errno));
        return -1;
    }

    if (fstat(fd, &st) == -1) {
        LOG_INFO("fstat: %s\n", strerror(errno));
        close(fd);
        return -1;
    }

    if (S_ISDIR(st.st_mode)) {
        LOG_INFO("File is a directory: `%s`\n", fname);
        close(fd);
        return -1;
    }

    return fd;
}

int32_t
file_close(int32_t fd) {
    if (close(fd) == -1) {
        LOG_INFO("close: %s\n", strerror(errno));
        return HTTP_ERR;
    }

    return HTTP_OK;
}

int32_t
file_stream(int32_t fd, int32_t offset, unsigned char *buf, int32_t len) {
    int32_t nread = 0;

    if ((nread = read(fd, (char *)buf, len)) == -1) {
        LOG_INFO("read: %s\n", strerror(errno));
    }

    return nread;
}

int32_t
file_size(int32_t fd) {
    struct stat st;

    // TODO: potentially close the file
    if (fstat(fd, &st) == -1) {
        LOG_INFO("fstat: %s\n", strerror(errno));
        return HTTP_ERR;
    }

    return st.st_size;
}
