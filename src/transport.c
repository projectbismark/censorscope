#include "transport.h"

#include <stdlib.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "util.h"

int transport_init(transport_t *transport,
                   const censorscope_options_t *options,
                   const char *module) {
    transport->L = luaL_newstate();
    if (!transport->L) {
        return -1;
    }
    luaL_openlibs(transport->L);

    char *filename = sprintf_malloc("%s/transports/%s.lua",
                                    options->luasrc_dir,
                                    module);
    if (!filename) {
        return -1;
    }
    if (luaL_dofile(transport->L, filename)) {
        fprintf(stderr,
                "Error loading transport: %s\n",
                luaL_checkstring(transport->L, -1));
        free(filename);
        transport_destroy(transport);
        return -1;
    }

    transport->options = options;

    free(filename);
    return 0;
}

int transport_download(transport_t *transport) {
    lua_getfield(transport->L, -1, "sync_sandbox");
    lua_pushstring(transport->L, transport->options->sandbox_dir);
    if (lua_pcall(transport->L, 1, 0, 0)) {
        fprintf(stderr,
                "Error downloading files: %s\n",
                luaL_checkstring(transport->L, -1));
        lua_pop(transport->L, 1);
        return -1;
    }
    return 0;
}

int transport_upload(transport_t *transport) {
    lua_getfield(transport->L, -1, "upload_results");
    lua_pushstring(transport->L, transport->options->results_dir);
    if (lua_pcall(transport->L, 1, 0, 0)) {
        fprintf(stderr,
                "Error uploading results: %s\n",
                luaL_checkstring(transport->L, -1));
        lua_pop(transport->L, 1);
        return -1;
    }
    return 0;
}

int transport_destroy(transport_t *transport) {
    lua_close(transport->L);
    return 0;
}
