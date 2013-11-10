#include "transport.h"

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

int transport_init(transport_t *transport, const char *module) {
    transport->L = luaL_newstate();
    if (!transport->L) {
        return -1;
    }
    luaL_openlibs(transport->L);

    char filename[255];
    snprintf(filename, sizeof(filename), "transports/%s.lua", module);

    if (luaL_dofile(transport->L, filename)) {
        fprintf(stderr,
                "Error loading transport: %s\n",
                luaL_checkstring(transport->L, -1));
        lua_close(transport->L);
        return -1;
    }

    return 0;
}

int transport_download(transport_t *transport) {
    lua_getfield(transport->L, -1, "sync_sandbox");
    if (lua_pcall(transport->L, 0, 0, 0)) {
        fprintf(stderr,
                "Error downloading files: %s\n",
                luaL_checkstring(transport->L, -1));
        return -1;
    }
    return 0;
}

int transport_upload(transport_t *transport) {
    lua_getfield(transport->L, -1, "upload_results");
    if (lua_pcall(transport->L, 0, 0, 0)) {
        fprintf(stderr,
                "Error uploading results: %s\n",
                luaL_checkstring(transport->L, -1));
        return -1;
    }
    return 0;
}

int transport_destroy(transport_t *transport) {
    lua_close(transport->L);
    return 0;
}
