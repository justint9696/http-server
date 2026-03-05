#ifndef _HTTP_FILE_IO_H
#define _HTTP_FILE_IO_H

#include <stdint.h>
#include <stdio.h>

/**
  * Opens a file by name.
  *
  * @param[in] fname        name of file to open
  *
  * @return the file descriptor on success or -1 on failure
  */
int32_t
file_open(const char *fname);

/**
  * Closes a file.
  *
  * @param[in] fd       the open file descriptor
  *
  * @return HTTP_OK on succes or HTTP_ERR on failure
  */
int32_t
file_close(int32_t fd);

/**
  * Streams the contents of a file into a buffer.
  *
  * @param[in] fd       the open file descriptor
  * @param[in] offset   the offset into the file
  * @param[out] buf     the destination buffer
  * @param[in] len      the size of the buffer
  *
  * @return the number of bytes read on success or -1 on failure
  */
int32_t
file_stream(int32_t fd, int32_t offset, unsigned char *buf, int32_t len);

/**
  * Determines the byte size of a file.
  *
  * @param[in] fd       the open file descriptor
  *
  * @return the file size on success or HTTP_ERR on failure
  */
int32_t
file_size(int32_t fd);

/**
  * Determines if a file exists.
  *
  * @param[in] fd       the open file descriptor
  *
  * @return HTTP_TRUE found or HTTP_FALSE otherwise
  */
int32_t
file_exists(const char *fname);

#endif // _HTTP_FILE_IO_H
