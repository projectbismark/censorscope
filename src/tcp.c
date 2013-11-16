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
    /* this breaks for ipv6, use inet_pton */
    if (inet_aton(ip, &sin.sin_addr) == 0) {
        lua_pushnil(L);
        lua_pushstring(L, "error invalid ip address");
        return 2;
    }

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
    lua_pushboolean(L, 1);
    lua_pushnil(L);
    return 2;
}
