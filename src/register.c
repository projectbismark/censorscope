#include "register.h"

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "dns.h"
#include "sandbox.h"
#include "util.h"

int run_in_sandbox(lua_State *L) {
    sandbox_t *sandbox = lua_touserdata(L, lua_upvalueindex(1));
    const char *module = luaL_checkstring(L, -1);
    char *filename = module_filename(module);

    if (sandbox_run(sandbox, filename, "luasrc/api.lua")) {
        return luaL_error(L, "error running in sandbox");
    }

    return 1;
}

int register_functions(sandbox_t *sandbox) {
    lua_register(sandbox->L, "dns_lookup", l_dns_lookup);

    lua_pushlightuserdata(sandbox->L, sandbox);
    lua_pushcclosure(sandbox->L, run_in_sandbox, 1);
    lua_setglobal(sandbox->L, "run_in_sandbox");

    return 0;
}
