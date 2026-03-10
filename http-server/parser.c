#include "http/http.h"
#include "http/logger.h"
#include "http/types.h"

#include <ctype.h>
#include <string.h>

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
    HDR_CONNECTION,
    HDR_ACCEPT,
    
    HDR_MAX
};

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

#define KEYWORD_MAX     10

const keyword_t HTTP_KEYWORDS[KEYWORD_MAX] = {
    { "UNKNOWN",            KW_REQUEST,         RQST_UNKNOWN },
    { "GET",                KW_REQUEST,         RQST_GET },
    { "POST",               KW_REQUEST,         RQST_POST },
    { "PUT",                KW_REQUEST,         RQST_PUT },
    { "DELETE",             KW_REQUEST,         RQST_DELETE },
    { "Host",               KW_HEADER,          HDR_HOST, },
    { "Content-Type",       KW_HEADER,          HDR_TYPE, },
    { "Content-Length",     KW_HEADER,          HDR_LENGTH },
    { "Connection",         KW_HEADER,          HDR_CONNECTION },
    { "Accept",             KW_HEADER,          HDR_ACCEPT },
};

static int32_t
strntoi(const char *s, int32_t len) {
    int ret = 0;
    while (s && *s != '\0' && --len >= 0) {
        ret = ((ret * 10) + (*s++ - '0'));
    }
    return ret;
}

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
    int32_t ret;
    int32_t state;
    int32_t idx = 0;
    int32_t i;
    header_t *hd = NULL;
    const keyword_t *keyword = NULL;

    enum _parser_state {
        PS_ERR = -1,
        PS_QUIT,
        PS_METHOD,
        PS_STATUS_CODE,
        PS_STATUS_VALUE,
        PS_TARGET,
        PS_VERSION,
        PS_HEADER_NAME,
        PS_HEADER_VALUE,
        PS_BODY,
    };

    memset(http, 0, sizeof(http_t));
    lx = (lexer_t) { .buf = data, .len = len, .pos = 0 };
    state = PS_METHOD;

    ret = HTTP_OK;
    while (ret && lexer_next(&lx, &tok) != HTTP_ERR) {
        if (tok.type == TOK_SPACE)
            continue;

        if (tok.type == TOK_EOF)
            break;

        switch (state) {
            case PS_METHOD:
                if (!strncmp(tok.str, "HTTP/", 
                            ((tok.len < (int)strlen("HTTP/")) 
                             ? tok.len : (int)strlen("HTTP/")))) {
                    http->type = MSG_RESPONSE;
                    memcpy(&http->version, &tok, sizeof(token_t));
                    state = PS_STATUS_CODE;
                } else {
                    for (i = 0; i < KEYWORD_MAX; i++) {
                        keyword = &HTTP_KEYWORDS[i];
                        if (keyword->type != KW_REQUEST)
                            continue;

                        if (!(tok.len - strlen(keyword->text))
                                && !strncmp(tok.str, keyword->text, tok.len)) {
                            switch (keyword->type) {
                                case KW_REQUEST:
                                    http->type = MSG_REQUEST;
                                    http->method = keyword->method;
                                    break;
                                default: break;
                            }
                            break;
                        }
                    }
                    state = ((i >= KEYWORD_MAX) ? PS_ERR : PS_TARGET);
                }
                break;
            case PS_STATUS_CODE:
                if (tok.type == TOK_CRLF) {
                    state = PS_ERR;
                    break;
                }

                http->rc = strntoi(tok.str, tok.len);
                state = PS_STATUS_VALUE;
                break;
            case PS_STATUS_VALUE:
                if (tok.type == TOK_CRLF) {
                    state = PS_ERR;
                    break;
                }

                // XXX: does this string really need to be stored?
                state = PS_HEADER_NAME;
                break;
            case PS_TARGET:
                if (tok.type == TOK_CRLF) {
                    state = PS_ERR;
                    break;
                }

                memcpy(&http->target, &tok, sizeof(token_t));
                state = PS_VERSION;
                break;
            case PS_VERSION:
                if (tok.type == TOK_CRLF) {
                    state = ((http->version.len > 0) ? PS_HEADER_NAME : PS_ERR);
                    break;
                }

                memcpy(&http->version, &tok, sizeof(token_t));
                break;
            case PS_HEADER_NAME:
                if (tok.type == TOK_CRLF) {
                    state = PS_BODY;
                    break;
                } else if (tok.type == TOK_COLON) {
                    state = PS_HEADER_VALUE;
                    break;
                }

                if ((idx = http->nheaders++) >= HEADER_MAX) {
                    LOG_INFO("Header count exceeds maximum length %d\n", idx);
                    state = PS_ERR;
                    break;
                }

                hd = &http->headers[idx];
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

                // XXX: this clutters logs
                // if (i >= KEYWORD_MAX) {
                //     LOG_INFO("Unhandled keyword: `%.*s`\n", tok.len, tok.str);
                // }
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
                    state = PS_QUIT;
                    break;
                }

                if (!http->body.len) {
                    memcpy(&http->body, &tok, sizeof(token_t));
                } else {
                    http->body.len += tok.len;
                }

                break;
            case PS_ERR:
                ret = HTTP_ERR;
                break;
            case PS_QUIT:
                LOG_INFO("Parser breakout initiated\n");
                ret = HTTP_ERR;
                break;
            default:
                LOG_ERROR("Unknown parser state `%d`\n", state);
                ret = HTTP_ERR;
                break;
        } // end switch
    } // end while


    if (state == PS_ERR) {
        if (http->type == MSG_REQUEST) {
            http->rc = HTTP_RES_BAD_REQUEST;
        }
        LOG_ERROR("Invalid HTTP message format\n");
        return HTTP_ERR;
    }

    if (http->type == MSG_REQUEST) {
        http->rc = HTTP_RES_OK;
    }

    switch (http->type) {
        case MSG_REQUEST:
            LOG_TRACE("HTTP info: %d %.*s %.*s %d\n",
                    http->method,
                    http->target.len, http->target.str,
                    http->version.len, http->version.str,
                    http->nheaders);

            for (i = 0; i < http->nheaders; i++) {
                hd = &http->headers[i];
                LOG_TRACE("   %.*s: %.*s\n",
                        hd->name.len, hd->name.str, 
                        hd->value.len, hd->value.str);
            }

            if (http->body.len && http->body.str) {
                LOG_TRACE("HTTP body: %.*s\n", http->body.len, http->body.str);
            }

            break;
        default: break;
    }

    return HTTP_OK;
}
