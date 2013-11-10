#include "register.h"

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "dns.h"
#include "sandbox.h"

int register_functions(sandbox_t *sandbox) {
    lua_register(sandbox->L, "dns_lookup", l_dns_lookup);
    return 0;
}
