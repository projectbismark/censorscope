#include "sandbox.h"

#include <stdlib.h>
#include <string.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#define BYTECODE_MAGIC_NUMBER 27

/* Copied from lauxlib.c */
static int panic (lua_State *L) {
    (void)L;  /* to avoid warnings */
    fprintf(stderr,
            "PANIC: unprotected error in call to Lua API (%s)\n",
            lua_tostring(L, -1));
    return 0;
}

/* This is an allocation function that artificially limits the pool of memory
 * available to scripts allocate.
 *
 * References:
 * http://www.lua.org/manual/5.1/manual.html#lua_Alloc
 * http://stackoverflow.com/a/9672205
 *
 */
static void *l_alloc_restricted(void *ud, void *ptr, size_t osize, size_t nsize) {
    size_t *available = (size_t *)ud;

    if (nsize == 0) {  /* free */
        free(ptr);
        *available += osize;  /* reclaim memory */
        return NULL;
    } else {  /* malloc */
        if (nsize > osize && *available < (nsize - osize)) {
            fprintf(stderr, "Out of memory!\n");
            return NULL;
        }
        ptr = realloc(ptr, nsize);
        if (ptr) {  /* reallocation successful? */
            if (nsize > osize) {
                *available -= (nsize - osize);
            } else {
                *available += (osize - nsize);
            }
        }
        return ptr;
    }
}

/* This is a debug hook that aborts the currently running script. We use it to
 * abort a script after running a certain number of instruction cycles.
 *
 * References:
 * http://www.lua.org/manual/5.1/manual.html#lua_hook
 * http://stackoverflow.com/a/11425017
 *
 */
static void exit_hook(lua_State *L, lua_Debug *ar) {
    if (ar->event != LUA_HOOKCOUNT) {
        return;
    }
    luaL_error(L, "instruction limit reached");
}

/* Check whether an input file is Lua bytecode. We
 * do not evaluate Lua bytecode because it can escape
 * the sandbox.
 *
 * Returns 1 if the script is valid and 0 if it is not.
 *
 */
static int is_valid_lua_script(const char *filename) {
    FILE *handle = fopen(filename, "r");
    if (!handle) {
        perror(filename);
        return 0;
    }

    int first_byte = fgetc(handle);
    if (fclose(handle)) {
        perror(filename);
        return 0;
    }
    if (first_byte == 0) {
        return 0;
    }
    if (first_byte == BYTECODE_MAGIC_NUMBER) {
        fprintf(stderr, "for security, we do not evaluate Lua bytecode");
        return 0;
    }
    return 1;
}

/* A new entry to the packages search path.
 *
 * Adapted from http://stackoverflow.com/a/4156038.
 *
 * Arguments:
 * - L is a Lua state.
 * - new_entry should be an element of packages.path, like "luasrc/?.lua".
 * Returns: 0 on success, -1 on failure.
 *
 */
static int prepend_package_path(lua_State *L, const char* new_entry) {
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "path");  // get field "path" from table at top of stack (-1)
    const char *cur_path = lua_tostring(L, -1);  // grab path string from top of stack
    int new_len = strlen(cur_path) + strlen(new_entry) + 1 + 1;
    char *new_path = malloc(new_len);
    if (!new_path) {
        return -1;
    }
    snprintf(new_path, new_len, "%s;%s", new_entry, cur_path);
    lua_pop(L, 1);
    lua_pushstring(L, new_path);  // push the new one
    free(new_path);
    lua_setfield(L, -2, "path");  // set the field "path" in table at -2 with value at top of stack
    lua_pop(L, 1);  // get rid of package table from top of stack
    return 0;
}

int sandbox_init(sandbox_t *sandbox,
                 const char *name,
                 size_t max_memory,
                 int max_instructions) {
    sandbox->available_memory = max_memory;
    sandbox->L = lua_newstate(l_alloc_restricted, &sandbox->available_memory);
    if (!sandbox->L) {
        fprintf(stderr, "Error calling luaL_newstate\n");
        return -1;
    }
    lua_atpanic(sandbox->L, &panic);
    lua_sethook(sandbox->L, exit_hook, LUA_MASKCOUNT, max_instructions);
    luaL_openlibs(sandbox->L);
    lua_pushstring(sandbox->L, name);
    lua_setglobal(sandbox->L, "SANDBOX_NAME");

    /* Add luasrc/ to the path so that the script we evaluate to obtain the
     * environment can easily reference files in the same directory. For
     * example, we often use luasrc/api.lua for environment, and it should be
     * able to import files from that directory. */
    if (prepend_package_path(sandbox->L, "luasrc/?.lua")) {
        fprintf(stderr, "Error setting packages.path\n");
        return -1;
    }
    return 0;
}

int sandbox_destroy(sandbox_t *sandbox) {
    lua_close(sandbox->L);
    return 0;
}

int sandbox_run(sandbox_t *sandbox,
                const char *filename,
                const char *environment) {
    if (!is_valid_lua_script(filename)) {
        return -1;
    }

    /* Load (but not evaluate) the code to run in the sandbox. */
    if (luaL_loadfile(sandbox->L, filename)) {
        fprintf(stderr, "%s\n", lua_tostring(sandbox->L, -1));
        return -1;
    }
    if (environment) {
        /* Load (but not evaluate) the code that creates the environment. */
        if (luaL_loadfile(sandbox->L, environment)) {
            fprintf(stderr, "%s", lua_tostring(sandbox->L, -1));
            return -1;
        }

        /* Evaluate the environment. (Its code is at the top of the stack.) This
         * removes the the code and replaces it with the environment's table. */
        if (lua_pcall(sandbox->L, 0, 1, 0)) {
            fprintf(stderr, "%s\n", lua_tostring(sandbox->L, -1));
            return -1;
        }
    } else {
        /* Create an empty environment. */
        lua_newtable(sandbox->L);
    }

    /* Set the environment to use for the sandboxed evaluation. This pops the
     * environment off the top of the stack. */
    if (lua_setfenv(sandbox->L, -2) != 1) {
        fprintf(stderr, "%s\n", lua_tostring(sandbox->L, -1));
        return -1;
    }
    /* Evaluate the sandbox function, which is now at the top of the stack. */
    if (lua_pcall(sandbox->L, 0, 1, 0) != 0) {
        fprintf(stderr,
                "error running %s: %s\n",
                filename,
                lua_tostring(sandbox->L, -1));
        return -1;
    }

    return 0;
}
