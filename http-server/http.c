#include "http/http.h"
#include "http/logger.h"
#include "http/types.h"

#include <ctype.h>
#include <string.h>
#include <sys/types.h>

enum _keyword_type {
    KW_REQUEST,
    KW_RESPONSE,
    KW_HEADER,
    KW_MAX,
};

enum _hdr_type {
    HDR_UNKNOWN = 0,
    HDR_TYPE,
    HDR_LENGTH,
    
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

#define KEYWORD_MAX     8

const keyword_t HTTP_KEYWORDS[KEYWORD_MAX] = {
    { "UNKNOWN",            KW_REQUEST,         RQST_UNKNOWN },
    { "GET",                KW_REQUEST,         RQST_GET },
    { "POST",               KW_REQUEST,         RQST_POST },
    { "PUT",                KW_REQUEST,         RQST_PUT },
    { "DELETE",             KW_REQUEST,         RQST_DELETE },
    { "Host",               KW_HEADER,          HDR_TYPE, },
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
                                rqst->type = keyword->type;
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
                    LOG_WARN("Header count exceeded maximum value %d\n", idx);
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
                        // TODO: do something with this...
                        break;
                    }
                }

                if (i >= KEYWORD_MAX) {
                    LOG_INFO("Bad keyword: `%.*s`\n", tok.len, tok.str);
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

    if (http->type != MSG_UNKNOWN) {
        LOG_DEBUG("HTTP info: %d %.*s %.*s %d\n",
                http->type,
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
    }

    return HTTP_OK;
}
