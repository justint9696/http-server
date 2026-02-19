#ifndef _HTTP_H
#define _HTTP_H

#include <stddef.h>
#include <stdint.h>

/**
 * HTTP Message Structure:
 *    1. Version and Request method or Outcome of the request (Response)
 *    2. Headers containing metadata describing the message
 *    3. Blank line marking the end of the metadata section
 *    4. The body containing data associated with the message
 */

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define HEADER_MAX  32

enum _http_method {
    RQST_UNKNOWN = 0,
    RQST_GET,
    RQST_POST,
    RQST_PUT,
    RQST_DELETE,

    RQST_MAX
};

enum _msg_type {
    MSG_UNKNOWN = 0,
    MSG_REQUEST,
    MSG_RESPONSE,

    MSG_MAX
};

enum _token_type {
    TOK_UNKNOWN = 0,
    TOK_WORD,
    TOK_SPACE,
    TOK_COLON,
    TOK_CRLF,
    TOK_EOF
};

typedef struct _token {
    char        *str;
    int32_t     len;
    int32_t     type;
} token_t;

typedef struct _header {
    token_t     name;
    token_t     value;
} header_t;

typedef struct _request {
    int32_t     type;
    token_t     target;
    token_t     version;
    int32_t     nheaders;
    header_t    headers[HEADER_MAX];
    token_t     body;
} request_t;

typedef struct _response {

} response_t;

typedef struct _packet {
    int32_t     type;
    request_t   request;
    response_t  response;
} http_t;

int32_t
http_parse_message(http_t *http, char *data, int32_t len);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _HTTP_H
