#include "tcp.h"

#include <event2/util.h>

#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define LUALIB
#include "lua.h"
#include "lauxlib.h"

int l_tcp_connect(lua_State *L) {
    evutil_socket_t sock;
    struct sockaddr_in sin;

    const char *ip = luaL_checkstring(L, 1);
    const int port = luaL_checkinteger(L, 2);

    memset(&sin, 0, sizeof(sin));

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = inet_addr(ip);
    sin.sin_port = htons(port);

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        lua_pushnil(L);
        lua_pushstring(L, "error creating socket");
        return 2;
    }

    if (connect(sock, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
        lua_pushnil(L);
        lua_pushstring(L, "error connecting to ip");
        evutil_closesocket(sock);
        return 2;
    }

    evutil_closesocket(sock);
    lua_pushstring(L, "success");
    lua_pushnil(L);
    return 2;
}
