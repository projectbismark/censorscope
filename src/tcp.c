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

    memset(&sin, 0, sizeof(sin));

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = inet_addr("127.0.0.1");
    sin.sin_port = htons(5555);

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;

    if (connect(sock, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
        fprintf(stdout, "%s\n", "failure");
        evutil_closesocket(sock);
        return 2;
    }
    
    evutil_closesocket(sock);
    fprintf(stdout, "%s\n", "success");
    return 2;
}
