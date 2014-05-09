#include "register.h"

#include <stdlib.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "dns.h"
#include "logging.h"
#include "tcp.h"
#include "http.h"
#include "options.h"
#include "sandbox.h"
#include "util.h"

int run_in_sandbox(lua_State *L) {
    censorscope_options_t *options = lua_touserdata(L, lua_upvalueindex(1));
    sandbox_t *sandbox = lua_touserdata(L, lua_upvalueindex(2));
    const char *module = luaL_checkstring(L, -1);
    char *filename = module_filename(options->sandbox_dir, module);
    if (!filename) {
        return luaL_error(L, "invalid module name");
    }

    char *api_filename = sprintf_malloc("%s/api.lua", options->luasrc_dir);
    if (!api_filename) {
        return luaL_error(L, "error allocating api_filename");
    }
    if (sandbox_run(sandbox, filename, api_filename)) {
        free(api_filename);
        free(filename);
        return luaL_error(L, "error running in sandbox");
    }

    free(api_filename);
    free(filename);
    return 1;
}

int l_log_error(lua_State *L) {
    const char *message = luaL_checkstring(L, -1);
    log_error("%s", message);
    return 0;
}

int l_log_info(lua_State *L) {
    const char *message = luaL_checkstring(L, -1);
    log_info("%s", message);
    return 0;
}

int l_log_debug(lua_State *L) {
    const char *message = luaL_checkstring(L, -1);
    log_debug("%s", message);
    return 0;
}

int register_functions(censorscope_options_t *options, sandbox_t *sandbox) {
    lua_register(sandbox->L, "dns_lookup", l_dns_lookup);
    lua_register(sandbox->L, "http_get", l_http_get);
    lua_register(sandbox->L, "tcp_connect", l_tcp_connect);

    lua_register(sandbox->L, "log_error", l_log_error);
    lua_register(sandbox->L, "log_info", l_log_info);
    lua_register(sandbox->L, "log_debug", l_log_debug);

    lua_register(sandbox->L, "pony_ping", pony_ping);
    lua_register(sandbox->L, "pony_traceroute", pony_traceroute);
    lua_register(sandbox->L, "pony_fasttraceroute", pony_fasttraceroute);
    lua_register(sandbox->L, "pony_dns", pony_dns);
    lua_register(sandbox->L, "pony_gethttp", pony_gethttp);

    lua_pushlightuserdata(sandbox->L, options);
    lua_pushlightuserdata(sandbox->L, sandbox);
    lua_pushcclosure(sandbox->L, run_in_sandbox, 2);
    lua_setglobal(sandbox->L, "run_in_sandbox");

    return 0;
}
