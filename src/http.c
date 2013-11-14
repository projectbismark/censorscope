#include "http.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <event2/bufferevent_ssl.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/http.h>

#define LUALIB
#include "lua.h"
#include "lauxlib.h"
#include "censorscope.h"

static void
http_request_done(struct evhttp_request *req, void *ctx)
{
  char buffer[256];
  int nread;

  if (req == NULL) {
    /* If req is NULL, it means an error occurred, but
     * sadly we are mostly left guessing what the error
     * might have been.  We'll do our best... */
    int printed_err = 0;
    int errcode = EVUTIL_SOCKET_ERROR();
    fprintf(stderr, "some request failed - no idea which one though!\n");

    if (! printed_err)
      fprintf(stderr, "socket error = %s (%d)\n",
        evutil_socket_error_to_string(errcode),
        errcode);
    return;
  }

  fprintf(stderr, "Response line: %d %s\n",
      evhttp_request_get_response_code(req),
      evhttp_request_get_response_code_line(req));

  while ((nread = evbuffer_remove(evhttp_request_get_input_buffer(req),
        buffer, sizeof(buffer)))
         > 0) {
    /* These are just arbitrary chunks of 256 bytes.
     * They are not lines, so we can't treat them as such. */
    fwrite(buffer, nread, 1, stdout);
  }
}

int l_http_get(lua_State *L) {
    int r;

    struct evhttp_uri *http_uri;
    const char *url, *scheme, *host, *path, *query;
    char uri[256];
    int port;

    struct bufferevent *bev;
    struct evhttp_connection *evcon;
    struct evhttp_request *req;
    struct evkeyvalq *output_headers;

    url = luaL_checkstring(L, 1);

    http_uri = evhttp_uri_parse(url);
    if (http_uri == NULL) {
        lua_pushnil(L);
        lua_pushstring(L, "error malformed url");
        return 2;
    }

    scheme = evhttp_uri_get_scheme(http_uri);
    if (scheme == NULL || (strcasecmp(scheme, "http") != 0)) {
        lua_pushnil(L);
        lua_pushstring(L, "error url must be http");
        return 2;
    }

    host = evhttp_uri_get_host(http_uri);
    if (host == NULL) {
        lua_pushnil(L);
        lua_pushstring(L, "error url must have a host");
        return 2;
    }

    port = evhttp_uri_get_port(http_uri);
    if (port == -1) {
        port = (strcasecmp(scheme, "http") == 0) ? 80 : 443;
    }

    path = evhttp_uri_get_path(http_uri);
    if (path == NULL) {
        path = "/";
    }

    query = evhttp_uri_get_query(http_uri);
    if (query == NULL) {
        snprintf(uri, sizeof(uri) - 1, "%s", path);
    } else {
        snprintf(uri, sizeof(uri) - 1, "%s?%s", path, query);
    }
    uri[sizeof(uri) - 1] = '\0';

    if (strcasecmp(scheme, "http") == 0) {
        bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
    }

    // For simplicity, we let DNS resolution block. Everything else should be
    // asynchronous though.
    evcon = evhttp_connection_base_bufferevent_new(base, NULL, bev,
                                                   host, port);
    if (evcon == NULL) {
        lua_pushnil(L);
        lua_pushstring(L, "evhttp_connection_base_bufferevent_new() failed\n");
        return 2;      
    }

    // Fire off the request
    req = evhttp_request_new(http_request_done, bev);
    if (req == NULL) {
        lua_pushnil(L);
        lua_pushstring(L, "evhttp_request_new() failed\n");
        return 2;      
    }

    output_headers = evhttp_request_get_output_headers(req);
    evhttp_add_header(output_headers, "Host", host);
    evhttp_add_header(output_headers, "Connection", "close");

    r = evhttp_make_request(evcon, req, EVHTTP_REQ_GET, uri);
    if (r != 0) {
        lua_pushnil(L);
        lua_pushstring(L, "evhttp_make_request() failed\n");
        return 2;      
    }

    evhttp_connection_free(evcon);
    lua_pushnil(L);
    return 2;
}
