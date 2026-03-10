#include "http/http.h"
#include "http/file_io.h"
#include "http/logger.h"
#include "http/types.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#define CONTENT_TYPE_MAX 5

static const struct {
    const char *ext;
    const char *type;
} CONTENT_TYPE_XREF[CONTENT_TYPE_MAX] = {
    { "json", "application/json" },
    { "js", "application/javascript" },
    { "html", "text/html" },
    { "php", "text/html" },
    { "css", "text/css" },
};

typedef int32_t (*response_callback_fn_t)(const http_t *, http_t *, const char *);

static int32_t
http_fmt_400_response(const http_t *rqst, http_t *rspn, const char *dirname);

static int32_t
http_fmt_404_response(const http_t *rqst, http_t *rspn, const char *dirname);

static int32_t
http_fmt_GET_response(const http_t *rqst, http_t *rspn, const char *dirname);

int32_t
http_fmt_response(const http_t *rqst, http_t *rspn, const char *dirname) {
    int ret = HTTP_OK;
    response_callback_fn_t callback_fn = NULL;

    // response callback table
    static const response_callback_fn_t HTTP_RESPONSE_CALLBACK[RQST_MAX] = {
        http_fmt_400_response,      // RQST_UNKNOWN
        http_fmt_GET_response,      // RQST_GET
        NULL,                       // RQST_POST
        NULL,                       // RQST_PUT
        NULL,                       // RQST_DELETE
    };
    
    memset(rspn, 0, sizeof(http_t));

    switch (rqst->rc) {
        case HTTP_RES_BAD_REQUEST:
            (void)http_fmt_400_response(rqst, rspn, dirname);
            return HTTP_OK;
        default: break;
    }

    // format response based on the request method
    if ((callback_fn = HTTP_RESPONSE_CALLBACK[rqst->method]) != NULL
            && !(ret = callback_fn(rqst, rspn, dirname))) {
        LOG_INFO("Failed to format response\n");
    } 

    if (!callback_fn || !ret) {
        if (!http_fmt_404_response(rqst, rspn, dirname)) {
            LOG_ERROR("Failed to format default response\n");
            return HTTP_ERR;
        }
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
http_fmt_404_response(const http_t *rqst, http_t *rspn, const char *dirname) {
    const char *body = NULL;

    LOG_DEBUG("Creating default 404 response\n");

    body =
        "<html>\r\n"
        "<head>\r\n"
        "<title>Page Not Found</title>\r\n"
        "</head>\r\n"
        "<body>\r\n"
        "<p>The page you requested could not be found.</p>\r\n"
        "</body>\r\n"
        "</html>\r\n";
    snprintf(rspn->buf, sizeof(rspn->buf),
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %zu\r\n"
            "\r\n"
            "%s",
            strlen(body),
            body);

    LOG_TRACE("%s\n", rspn->buf);
    rspn->len = strlen(rspn->buf);
    rspn->rc = HTTP_RES_NOT_FOUND;

    return HTTP_OK;
}

static int32_t
http_fmt_400_response(const http_t *rqst, http_t *rspn, const char *dirname) {
    const char *body = NULL;

    LOG_DEBUG("Creating default 400 response\n");

    body =
        "<html>\r\n"
        "<head>\r\n"
        "<title>Bad Request</title>\r\n"
        "</head>\r\n"
        "<body>\r\n"
        "<p>400 Bad Request.</p>\r\n"
        "</body>\r\n"
        "</html>\r\n";
    snprintf(rspn->buf, sizeof(rspn->buf),
            "HTTP/1.1 400 Bad Request\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %zu\r\n"
            "\r\n"
            "%s",
            strlen(body),
            body);

    rspn->len = strlen(rspn->buf);
    rspn->rc = HTTP_RES_BAD_REQUEST;

    return HTTP_OK;
}

static int32_t
http_fmt_GET_response(const http_t *rqst, http_t *rspn, const char *dirname) {
    const header_t *hd = NULL;
    char timestr[64];
    char fpath[FILENAME_MAX];
    char *p = NULL;
    const char *type = NULL;
    int32_t i;
    int32_t nread;
    int32_t size;
    int32_t fd = -1;
    int32_t offset;

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
        "index.php",
        "default.html"
    };

    static const int32_t len = sizeof(DEFAULT_FILES) / sizeof(char *);

    if (rqst->target.len == 1 && *rqst->target.str == '/') {
        // look for an index file in the root dir
        LOG_DEBUG("Searching for index file\n");
        for (i = 0; i < len; i++) {
            snprintf(fpath, sizeof(fpath), "%s/%s",
                    dirname, DEFAULT_FILES[i]);
            if (file_exists(fpath) == HTTP_TRUE) {
                if ((fd = file_open(fpath)) == -1) {
                    return http_fmt_400_response(rqst, rspn, dirname);
                }
                break;
            }
        }

        if (len == i) {
            LOG_INFO("Could not find default file in directory `%s`\n", dirname);
            return http_fmt_404_response(rqst, rspn, dirname);
        }
    } else {
        snprintf(fpath, sizeof(fpath), "%s%.*s",
                dirname, rqst->target.len, rqst->target.str);
        if ((fd = file_open(fpath)) == -1) {
            return http_fmt_404_response(rqst, rspn, dirname);
        }
    }

    if (!(size = file_size(fd))) {
        LOG_INFO("Could not determine file size\n");
        return http_fmt_400_response(rqst, rspn, dirname);
    }

    if ((p = strchr(fpath, '.')) == NULL) {
        LOG_WARN("Unable to determine content type\n");
        return http_fmt_400_response(rqst, rspn, dirname);
    }

    p++;
    type = NULL;
    for (i = 0; i < CONTENT_TYPE_MAX; i++) {
        if (!strcmp(CONTENT_TYPE_XREF[i].ext, p)) {
            type = CONTENT_TYPE_XREF[i].type;
            break;
        }
    }

    if (!type) {
        LOG_WARN("Unable to determine content type\n");
        return http_fmt_400_response(rqst, rspn, dirname);
    }

    http_date_now(timestr, sizeof(timestr));
    snprintf(rspn->buf, sizeof(rspn->buf),
             "HTTP/1.1 200 OK\r\n"
             "Server: HTTP Server\r\n"
             "Date: %s\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %d\r\n"
             "\r\n",
             timestr, type, size);

    rspn->len = strlen(rspn->buf);
    rspn->rc = HTTP_RES_OK;

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
