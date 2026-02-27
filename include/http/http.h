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

enum _http_res_info {
    HTTP_RES_CONTINUE = 100,
    HTTP_RES_SWITCH_PROTO,
    HTTP_PROCESSING,
    HTTP_EARLY_HINT
};

enum _http_res_success {
    HTTP_RES_OK = 200,
    HTTP_RES_CREATED,
    HTTP_RES_ACCEPTED,
    HTTP_RES_INFO,
    HTTP_RES_NO_CONTENT,
    HTTP_RES_RESET_CONTENT,
    HTTP_RES_PARTIAL_CONTENT,
    HTTP_RES_MULTI_STATUS
};

enum _http_res_client_error {
    HTTP_RES_BAD_REQUEST = 400,
    HTTP_RES_UNAUTHORIZED,
    HTTP_RES_PAYMENT_REQUIRED,
    HTTP_RES_FORBIDDEN,
    HTTP_RES_NOT_FOUND,
    HTTP_RES_METHOD_NOT_ALLOWED,
    HTTP_RES_NOT_ACCEPTABLE,
    HTTP_RES_PROXY_AUTHENTICATION_REQUIRED,
    HTTP_RES_REQUEST_TIMEOUT,
};

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
    int32_t     type;
    token_t     name;
    token_t     value;
} header_t;

typedef struct _request {
    int32_t     method;
    token_t     target;
    token_t     version;
    int32_t     nheaders;
    header_t    headers[HEADER_MAX];
    token_t     body;
} request_t;

typedef struct _response {
    int32_t     nheaders;
    header_t    headers[HEADER_MAX];
    token_t     body;
    int32_t     len;
    char        buf[8192];
} response_t;

typedef struct _http {
    int32_t     type;
    request_t   request;
    response_t  response;
} http_t;

/**
  * Parses data from an HTTP message.
  *
  * @param[in] http         a refernce to an http structure
  * @param[in] data         the http packet data
  * @param[in] len          the length of the packet data
  *
  * @return HTTP_OK on success or HTTP_ERR on failure
  */
int32_t
http_parse_message(http_t *http, char *data, int32_t len);

/**
  * Formats an HTTP response message.
  *
  * @param[in] http         a refernce to an http structure
  * @param[in] rc           the return code of the parser
  * @param[in] dirname      the file serving directory
  *
  * @return HTTP_OK on success or HTTP_ERR on failure
  */
int32_t
http_fmt_response(http_t *http, int32_t rc, const char *dirname);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _HTTP_H
