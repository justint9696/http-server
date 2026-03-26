#ifndef _HTTP_SOCKET_H
#define _HTTP_SOCKET_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef int32_t socket_t;

/*
 * Creates a socket and listens for incoming connections.
 * 
 * @param sck[in]               a reference to a socket structure
 * @param portno[in]            the port number of the endpoint
 *
 * @returns HTTP_OK on success or HTTP_ERR on failure
 */
int32_t
socket_bind(socket_t *sck, int32_t portno);

/*
 * Creates a socket and connects to the given IP address.
 * 
 * @note if the ip address is null, the routine will use 127.0.0.1 (localhost)
 *
 * @param sck[in]               a reference to a socket structure
 * @param ip[in]                the ip address of the endpoint
 * @param portno[in]            the port number of the endpoint
 *
 * @returns HTTP_OK on success or HTTP_ERR on failure
 */
int32_t
socket_connect(socket_t *sck, const char *ip_addr, int32_t portno);

/*
 * Closes an open socket.
 *
 * @param sck[in]               a reference to a socket structure
 *
 * @returns HTTP_OK on success or HTTP_ERR on failure
 */
int32_t
socket_close(socket_t *sck);

/*
 * Wait for a client to connect and accepts the connection.
 *
 * @param sck[in]               a reference to a socket structure
 * @param fd[out]               a reference for the client file descriptor
 *
 * @returns HTTP_OK on success or HTTP_ERR on failure
 */
int32_t
socket_accept(socket_t *sck, int32_t *fd);

/*
 * Receives packet data from a client.
 *
 * @param sck[in]               a reference to a socket structure
 * @param data[out]             the desination buffer
 * @param len[in]               the length of the buffer
 *
 * @returns the number of bytes received on success or -1 on failure
 */
int32_t
socket_recv(socket_t *sck, void *data, int32_t len);

/*
 * Sends packet data to a client.
 *
 * @param sck[in]               a reference to a socket structure
 * @param data[in]              the desination buffer
 * @param len[in]               the length of the buffer
 *
 * @returns HTTP_OK On successs or HTTP_ERR on failure
 */
int32_t
socket_send(socket_t *sck, const void *data, int32_t len);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _HTTP_SOCKET_H
