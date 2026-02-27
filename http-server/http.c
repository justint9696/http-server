#include "http/http.h"
#include "http/file_io.h"
#include "http/logger.h"
#include "http/types.h"

#include <ctype.h>
#include <errno.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>

enum _keyword_type {
    KW_REQUEST,
    KW_RESPONSE,
    KW_HEADER,
    KW_MAX,
};

enum _hdr_type {
    HDR_UNKNOWN = 0,
    HDR_HOST,
    HDR_TYPE,
    HDR_LENGTH,
    
    HDR_MAX
};

typedef int32_t (*response_callback_fn_t)(http_t *, const char *);

typedef struct _keyword {
    char        *text;
    int32_t     type;
    int32_t     method;
} keyword_t;

typedef struct _lexer {
    char        *buf;
    int32_t     len;
    int32_t     pos;
} lexer_t;

#define KEYWORD_MAX     8

const keyword_t HTTP_KEYWORDS[KEYWORD_MAX] = {
    { "UNKNOWN",            KW_REQUEST,         RQST_UNKNOWN },
    { "GET",                KW_REQUEST,         RQST_GET },
    { "POST",               KW_REQUEST,         RQST_POST },
    { "PUT",                KW_REQUEST,         RQST_PUT },
    { "DELETE",             KW_REQUEST,         RQST_DELETE },
    { "Host",               KW_HEADER,          HDR_HOST, },
    { "Content-Type",       KW_HEADER,          HDR_TYPE, },
    { "Content-Length",     KW_HEADER,          HDR_LENGTH },
};

static int32_t
lexer_next(lexer_t *lx, token_t *tok) {
    char *p = NULL;

    memset(tok, 0, sizeof(token_t));
    if (lx->pos >= lx->len) {
        tok->type = TOK_EOF;
        return HTTP_ERR;
    }

    p = &lx->buf[lx->pos];
    tok->str = p;

    if (*p == '\r') {
        if (lx->pos + 1 < lx->len && lx->buf[lx->pos + 1] == '\n') {
            tok->type = TOK_CRLF;
            lx->pos += 2;
            tok->len = 2;
            return HTTP_OK;
        }

        return HTTP_ERR;
    }

    if (*p == '\n') {
        tok->len = 1;
        tok->type = TOK_CRLF;
    } else if (*p == ':') {
        tok->len = 1;
        tok->type = TOK_COLON;
    }  else if (isspace(*p)) {
        tok->len = 1;
        tok->type = TOK_SPACE;
    }

    if (tok->len == 1) {
        lx->pos++;
        p++;
        return HTTP_OK;
    }

    while (!isspace(*p) && *p != ':' && *p != '\0') {
        lx->pos++;
        tok->len++;
        p++;
    }

    return HTTP_OK;
}

int32_t
http_parse_message(http_t *http, char *data, int32_t len) {
    lexer_t lx;
    token_t tok;
    int32_t state;
    int32_t idx = 0;
    int32_t i;
    header_t *hd = NULL;
    request_t *rqst = NULL;
    const keyword_t *keyword = NULL;

    enum _parser_state {
        PS_ERR = -1,
        PS_METHOD,
        PS_TARGET,
        PS_VERSION,
        PS_HEADER_NAME,
        PS_HEADER_VALUE,
        PS_BODY,
    };

    memset(http, 0, sizeof(http_t));
    lx = (lexer_t) { .buf = data, .len = len, .pos = 0 };
    state = PS_METHOD;

    rqst = &http->request;
    while (lexer_next(&lx, &tok) != HTTP_ERR) {
        if (tok.type == TOK_SPACE)
            continue;

        if (tok.type == TOK_EOF)
            break;

        switch (state) {
            case PS_METHOD:
                for (i = 0; i < KEYWORD_MAX; i++) {
                    keyword = &HTTP_KEYWORDS[i];
                    if (keyword->type != KW_REQUEST)
                        continue;

                    if (!strncmp(tok.str, keyword->text, tok.len)) {
                        switch (keyword->type) {
                            case KW_REQUEST:
                                http->type = MSG_REQUEST;
                                rqst->method = keyword->method;
                                break;
                            case KW_RESPONSE:
                                http->type = MSG_RESPONSE;
                                break;
                            default: break;
                        }
                        break;
                    }
                }

                // unable to determine if it's a request or response
                state = ((i >= KEYWORD_MAX) ? PS_ERR : PS_TARGET);
                break;
            case PS_TARGET:
                memcpy(&rqst->target, &tok, sizeof(token_t));
                state = PS_VERSION;
                break;
            case PS_VERSION:
                // consume new line
                if (tok.type == TOK_CRLF) {
                    state = PS_HEADER_NAME;
                    break;
                }

                memcpy(&rqst->version, &tok, sizeof(token_t));
                break;
            case PS_HEADER_NAME:
                if (tok.type == TOK_CRLF) {
                    state = PS_BODY;
                    break;
                } else if (tok.type == TOK_COLON) {
                    state = PS_HEADER_VALUE;
                    break;
                }

                if ((idx = rqst->nheaders++) >= HEADER_MAX) {
                    LOG_INFO("Header count exceeds maximum length %d\n", idx);
                    state = PS_ERR;
                    break;
                }

                hd = &rqst->headers[idx];
                memcpy(&hd->name, &tok, sizeof(token_t));

                for (i = 0; i < KEYWORD_MAX; i++) {
                    keyword = &HTTP_KEYWORDS[i];
                    if (keyword->type != KW_HEADER)
                        continue;

                    if (!strncmp(tok.str, keyword->text, tok.len)) {
                        hd->type = keyword->method;
                        break;
                    }
                }

                if (i >= KEYWORD_MAX) {
                    LOG_INFO("Unknown keyword: `%.*s`\n", tok.len, tok.str);
                }
                break;
            case PS_HEADER_VALUE:
                if (tok.type == TOK_CRLF) {
                    state = PS_HEADER_NAME;
                    break;
                }

                if (!hd->value.len) {
                    memcpy(&hd->value, &tok, sizeof(token_t));
                } else {
                    hd->value.len += tok.len;
                }

                break;
            case PS_BODY:
                if (tok.type == TOK_EOF) {
                    state = PS_ERR;
                    break;
                }

                if (!rqst->body.len) {
                    memcpy(&rqst->body, &tok, sizeof(token_t));
                } else {
                    rqst->body.len += tok.len;
                }

                break;
            default: break;
        }
    }

    switch (http->type) {
        case MSG_REQUEST:
            LOG_DEBUG("HTTP info: %d %.*s %.*s %d\n",
                    http->request.method,
                    rqst->target.len, rqst->target.str,
                    rqst->version.len, rqst->version.str,
                    rqst->nheaders);

            for (i = 0; i < rqst->nheaders; i++) {
                hd = &rqst->headers[i];
                LOG_DEBUG("   %.*s: %.*s\n",
                        hd->name.len, hd->name.str, 
                        hd->value.len, hd->value.str);
            }

            LOG_DEBUG("HTTP body: %.*s\n", rqst->body.len, rqst->body.str);
            break;
        default: break;
    }

    return HTTP_OK;
}

static inline int32_t
http_date_now(char *buf, int32_t len) {
    time_t ts;

    time(&ts);
    strftime(buf, len, "%c", localtime(&ts));

    return HTTP_OK;
}
static int32_t
http_fmt_default_response(http_t *http, const char *dirname) {
    char timestr[64];
    response_t *rspn = NULL;

    LOG_DEBUG("Creating default 400 response\n");
    rspn = &http->response;

    http_date_now(timestr, sizeof(timestr));
    snprintf(rspn->buf, sizeof(rspn->buf),
             "HTTP/1.1 400 Bad Request\n"
             "Server: HTTP Server\n"
             "Date: %s\n"
             "\n",
             timestr);

    rspn->len = strlen(rspn->buf);
    return HTTP_OK;
}

static int32_t
http_fmt_get_response(http_t *http, const char *dirname) {
    request_t *rqst = NULL;
    response_t *rspn = NULL;
    header_t *hd = NULL;
    char timestr[64];
    char fpath[FILENAME_MAX];
    int32_t i;
    int32_t nread;
    int32_t size;
    int32_t fd = -1;
    int32_t offset;
    int32_t len;

    rqst = &http->request;
    rspn = &http->response;

    // TODO: should probably do something with these
    for (i = 0; i < rqst->nheaders; i++) {
        hd = &rqst->headers[i];
        switch (hd->type) {
            default: break;
        }
    }

    static const char *DEFAULT_FILES[] = {
        "index.html",
        "index.htm",
        "index.php"
    };

    len = sizeof(DEFAULT_FILES) / sizeof(char *);

    if (rqst->target.len == 1 && *rqst->target.str == '/') {
        // look for an index file in the root dir
        for (i = 0; i < len; i++) {
            snprintf(fpath, sizeof(fpath), "%s/%s",
                    dirname, DEFAULT_FILES[i]);
            if ((fd = file_open(fpath)) != -1) {
                LOG_DEBUG("Found %s\n", fpath);
                break;
            }
        }

        if (len == i) {
            LOG_INFO("Could not find default file in directory `%s`\n", dirname);
            return http_fmt_default_response(http, dirname);
        }
    } else {
        snprintf(fpath, sizeof(fpath), "%s%.*s",
                dirname, rqst->target.len, rqst->target.str);
        if ((fd = file_open(fpath)) == -1) {
            LOG_INFO("Bad file: `%s`\n", fpath);
            return http_fmt_default_response(http, dirname);
        }
    }

    if (!(size = file_size(fd))) {
        LOG_INFO("Could not determine file size\n");
        return http_fmt_default_response(http, dirname);
    }

    http_date_now(timestr, sizeof(timestr));
    snprintf(rspn->buf, sizeof(rspn->buf),
             "HTTP/1.1 200 OK\n"
             "Server: HTTP Server\n"
             "Date: %s\n"
             "Content-Length: %d\n"
             "\n",
             timestr, size);

    rspn->len = strlen(rspn->buf);

    if ((int)(sizeof(rspn->buf) - rspn->len) < size) {
        // TODO: stream the file to client
        LOG_INFO("File size exceeds the 8kb limit\n");
    }

    offset = 0;
    while ((nread = file_stream(
                    fd,
                    offset,
                    (unsigned char *)(rspn->buf + rspn->len),
                    sizeof(rspn->buf) - rspn->len)) > 0) {
        offset += nread;
        rspn->len += nread;
    }

    (void)file_close(fd);

    return HTTP_OK;
}

int32_t
http_fmt_response(http_t *http, int32_t rc, const char *dirname) {
    request_t *rqst = NULL;
    response_t *rspn = NULL;
    response_callback_fn_t callback_fn = NULL;

    // response callback table
    static const response_callback_fn_t HTTP_RESPONSE_CALLBACK[RQST_MAX] = {
        http_fmt_default_response,      // RQST_UNKNOWN
        http_fmt_get_response,          // RQST_GET
        http_fmt_default_response,      // RQST_POST
        http_fmt_default_response,      // RQST_PUT
        http_fmt_default_response,      // RQST_DELETE
    };
    
    rqst = &http->request;
    rspn = &http->response;

    memset(rspn, 0, sizeof(response_t));

    // format response based on the request method
    if ((callback_fn = HTTP_RESPONSE_CALLBACK[rqst->method]) == NULL) {
        LOG_WARN("Unknown method type: `%d`\n");
        return HTTP_OK;
    }

    if (!callback_fn(http, dirname)) {
        LOG_ERROR("Failed to format response\n");
        return HTTP_ERR;
    }
    
    return HTTP_OK;
}
