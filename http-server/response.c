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

typedef int32_t (*response_callback_fn_t)(http_t *, const char *);

static int32_t
http_fmt_400_response(http_t *http, const char *dirname);

static int32_t
http_fmt_404_response(http_t *http, const char *dirname);

static int32_t
http_fmt_GET_response(http_t *http, const char *dirname);

int32_t
http_fmt_response(http_t *http, int32_t rc, const char *dirname) {
    int ret = HTTP_OK;
    request_t *rqst = NULL;
    response_t *rspn = NULL;
    response_callback_fn_t callback_fn = NULL;

    // response callback table
    static const response_callback_fn_t HTTP_RESPONSE_CALLBACK[RQST_MAX] = {
        http_fmt_400_response,      // RQST_UNKNOWN
        http_fmt_GET_response,      // RQST_GET
        NULL,                       // RQST_POST
        NULL,                       // RQST_PUT
        NULL,                       // RQST_DELETE
    };
    
    rqst = &http->request;
    rspn = &http->response;

    memset(rspn, 0, sizeof(response_t));

    if (!rc) {
        (void)http_fmt_400_response(http, dirname);
        return HTTP_OK;
    }

    // format response based on the request method
    if ((callback_fn = HTTP_RESPONSE_CALLBACK[rqst->method]) != NULL
            && !(ret = callback_fn(http, dirname))) {
        LOG_INFO("Failed to format response\n");
    } 

    if (!callback_fn || !ret) {
        if (!http_fmt_404_response(http, dirname)) {
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
http_fmt_404_response(http_t *http, const char *dirname) {
    const char *body = NULL;
    response_t *rspn = NULL;

    LOG_DEBUG("Creating default 404 response\n");
    rspn = &http->response;

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
    return HTTP_OK;
}

static int32_t
http_fmt_400_response(http_t *http, const char *dirname) {
    const char *body = NULL;
    response_t *rspn = NULL;

    LOG_DEBUG("Creating default 400 response\n");
    rspn = &http->response;

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
    return HTTP_OK;
}

static int32_t
http_fmt_GET_response(http_t *http, const char *dirname) {
    request_t *rqst = NULL;
    response_t *rspn = NULL;
    header_t *hd = NULL;
    char timestr[64];
    char fpath[FILENAME_MAX];
    char *p = NULL;
    const char *type = NULL;
    int32_t i;
    int32_t nread;
    int32_t size;
    int32_t fd = -1;
    int32_t offset;

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

    static const int32_t len = sizeof(DEFAULT_FILES) / sizeof(char *);

    if (rqst->target.len == 1 && *rqst->target.str == '/') {
        // look for an index file in the root dir
        for (i = 0; i < len; i++) {
            snprintf(fpath, sizeof(fpath), "%s/%s",
                    dirname, DEFAULT_FILES[i]);
            if ((fd = file_open(fpath)) != -1) {
                LOG_DEBUG("Found `%s`\n", fpath);
                break;
            }
        }

        if (len == i) {
            LOG_INFO("Could not find default file in directory `%s`\n", dirname);
            return http_fmt_404_response(http, dirname);
        }
    } else {
        snprintf(fpath, sizeof(fpath), "%s%.*s",
                dirname, rqst->target.len, rqst->target.str);
        if ((fd = file_open(fpath)) == -1) {
            LOG_INFO("Bad file `%s`\n", fpath);
            return http_fmt_404_response(http, dirname);
        }
    }

    if (!(size = file_size(fd))) {
        LOG_INFO("Could not determine file size\n");
        return http_fmt_400_response(http, dirname);
    }

    if ((p = strchr(fpath, '.')) == NULL) {
        LOG_WARN("Unable to determine content type\n");
        return http_fmt_400_response(http, dirname);
    }

    p++;
    type = NULL;
    for (i = 0; i < CONTENT_TYPE_MAX; i++) {
        if (strcmp(CONTENT_TYPE_XREF[i].ext, p))
            continue;

        type = CONTENT_TYPE_XREF[i].type;
    }

    if (!type) {
        LOG_WARN("Unable to determine content type\n");
        return http_fmt_400_response(http, dirname);
    }

    http_date_now(timestr, sizeof(timestr));
    snprintf(rspn->buf, sizeof(rspn->buf),
             "HTTP/1.1 200 OK\n"
             "Server: HTTP Server\n"
             "Date: %s\n"
             "Content-Type: %s\n"
             "Content-Length: %d\n"
             "\n",
             timestr, type, size);

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
